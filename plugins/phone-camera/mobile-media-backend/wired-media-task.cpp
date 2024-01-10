#include "wired-media-task.h"
#include "application.h"
#include "ios/screenmirror/packet.h"
#include "c-util.h"
#include "android-adb/android-adb-lib.h"

static QString serialNumber(QString path)
{
	path = path.left(path.lastIndexOf('#'));
	return path.mid(path.lastIndexOf('#') + 1);
}

WiredMediaTask::WiredMediaTask(PhoneType type, QObject *parent) : BaseMediaTask(type, parent), m_phoneType(type)
{
	m_scanTimer.setInterval(100);
	m_scanTimer.setSingleShot(false);
	connect(&m_scanTimer, &QTimer::timeout, this, &WiredMediaTask::onScanTimerTimeout);

	m_usbmuxdConnectTimer.setInterval(100);
	m_usbmuxdConnectTimer.setSingleShot(false);
	connect(&m_usbmuxdConnectTimer, &QTimer::timeout, this, [=]() {
		int fd = usbmuxd_connect(m_iOSDeviceHandle, 2345);
		if (fd == -ENODEV)
			stopTask(false);
		else if (fd > 0) {
			m_usbmuxdConnectTimer.stop();
			m_usbmuxdFD = fd;
			add_exceptions(m_finalId.toUtf8().data());
			add_media_callback(m_finalId.toUtf8().data(), WiredMediaTask::deviceData, WiredMediaTask::deviceLost, this);
		}
	});
}

WiredMediaTask::~WiredMediaTask()
{
	stopTask(true);
}

void WiredMediaTask::setExpectedDevice(QString device)
{
	do {
		if (m_expectedDevice.isEmpty())
			break;

		if (device == "auto")
			return;
		else {
			if (m_expectedDevice == "auto") //todo check auto device is same with target device
				break;
			else {
				if (m_expectedDevice == device)
					return;
				else
					break;
			}
		}
	} while (1);

	stopTask(true);
	m_expectedDevice = device;
	m_scanTimer.start();
}

void WiredMediaTask::startTask(QString path, uint32_t handle)
{
	if (m_taskStarted)
		return;

	qDebug() << "WiredMediaTask start ====> " << path;

	m_scanTimer.stop();
	m_connectedDevice = path;
	m_finalId = path;
	m_iOSDeviceHandle = handle;
	switch (m_phoneType) {
	case PhoneType::iOS:
	case PhoneType::iOSCamera:
		m_finalId.replace('-', "").toUpper();
		break;
	case PhoneType::Android:
		m_finalId = serialNumber(m_connectedDevice);
		break;
	default:
		break;
	}
	App()->updateRunningDevices(true, path);

	m_mediaCache.clear();
	if (m_phoneType == PhoneType::iOSCamera) {
		m_usbmuxdConnectTimer.start();
	} else if (m_phoneType == PhoneType::iOS || m_phoneType == PhoneType::Android)
		add_media_callback(m_finalId.toUtf8().data(), WiredMediaTask::deviceData, WiredMediaTask::deviceLost, this);
	else if (m_phoneType == PhoneType::AndroidADB) {
		start_android_adb_server(
			m_connectedDevice.toUtf8().data(),
			[this](media_status status) {
				emit mediaState(status);
				if (status == media_status::Media_STOP)
					QMetaObject::invokeMethod(this, "stopTask", Q_ARG(bool, false));
			},
			[this](uint8_t *data, size_t size, int64_t ts, bool isVideo) { emit mediaData((char *)data, size, ts, isVideo); });
	}

	m_taskStarted = true;
}

void WiredMediaTask::stopTask(bool finalStop)
{
	if (!m_taskStarted)
		return;

	qDebug() << "WiredMediaTask stop...";

	if (m_usbmuxdConnectTimer.isActive())
		m_usbmuxdConnectTimer.stop();

	if (m_usbmuxdFD) {
		usbmuxd_disconnect(m_usbmuxdFD);
		m_usbmuxdFD = -1;
	}

	remove_media_callback(m_finalId.toUtf8().data());

	if (m_phoneType == PhoneType::AndroidADB)
		stop_android_adb_server();

	App()->updateRunningDevices(false, m_connectedDevice);
	m_connectedDevice = QString();
	if (!finalStop) {
		m_scanTimer.start();
		qDebug() << "start device scan timer";
	}

	m_taskStarted = false;
}

void WiredMediaTask::onScanTimerTimeout()
{
	if (m_phoneType == PhoneType::Android || m_phoneType == PhoneType::AndroidADB) {
		auto devices = m_phoneType == PhoneType::Android ? App()->androidAOADevices() : App()->androidADBDevices();
		for (auto iter = devices.begin(); iter != devices.end(); iter++) {
			if (App()->deviceInUse(*iter))
				continue;

			auto validDevice = [this](QString device) {
				if (m_phoneType == PhoneType::Android)
					return device.contains(serialNumber(m_expectedDevice));
				else
					return device == m_expectedDevice;
			};

			if (m_expectedDevice == "auto" || validDevice(*iter)) {
				startTask(*iter);
				break;
			}
		}
	} else if (m_phoneType == PhoneType::iOS || m_phoneType == PhoneType::iOSCamera) {
		auto devices = App()->iOSDevices();
		for (auto iter = devices.begin(); iter != devices.end(); iter++) {
			if (App()->deviceInUse(iter.key()))
				continue;

			if (m_expectedDevice == "auto" || m_expectedDevice == iter.key()) {
				// try connect
				startTask(iter.key(), iter.value().second);
				break;
			}
		}
	}
}

void WiredMediaTask::deviceDataInternal(char *buf, int size)
{
	if (m_phoneType == PhoneType::iOSCamera) // todo
		return;

	m_mediaCache.append(buf, size);
	while (true) {
		int headerSize = sizeof(media_header);
		if (m_mediaCache.size() < headerSize)
			break;

		media_header header = {0};
		memcpy(&header, m_mediaCache.data(), headerSize);
		if (m_mediaCache.size() < headerSize + header.payload_size)
			break;

		auto media = m_mediaCache.mid(headerSize, header.payload_size);
		if (header.type == 2) {
			int type = 0;
			memcpy(&type, media.data(), sizeof(int));
			emit mediaState(type == 0 ? media_status::Media_START : media_status::Media_STOP);
		} else
			emit mediaData(media.data(), media.size(), header.timestamp, header.type == 0);
		m_mediaCache.remove(0, headerSize + header.payload_size);
	}
}

void WiredMediaTask::deviceData(char *buf, int size, void *cb_data)
{
	WiredMediaTask *s = (WiredMediaTask *)cb_data;
	s->deviceDataInternal(buf, size);
}

void WiredMediaTask::deviceLost(void *cb)
{
	WiredMediaTask *s = (WiredMediaTask *)cb;
	QMetaObject::invokeMethod(s, [=]() { s->stopTask(false); });
}
