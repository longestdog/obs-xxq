#include "driver-helper.h"
#include <qstandardpaths.h>
#include <qfile.h>
#include <qdebug.h>
#include <qapplication.h>
#include <qdatetime.h>
#include <qthread.h>
#include <pthread.h>
#include <libusb-1.0/libusb.h>
#include <Setupapi.h>
#include <Devpkey.h>
#include "c-util.h"
#include "application.h"
#include "android-adb/android-adb-lib.h"

static QTimer *ADBCheckTimer = nullptr;

extern "C" {
QRecursiveMutex mutex;

void lock_install()
{
	if (QThread::currentThread() == App()->thread())
		ADBCheckTimer->stop();

	mutex.lock();
}

void unlock_install()
{
	mutex.unlock();

	if (QThread::currentThread() == App()->thread())
		ADBCheckTimer->start();
}
}

DriverHelper::DriverHelper(QObject *parent) : QObject(parent)
{
	if (!ADBCheckTimer) {
		ADBCheckTimer = new QTimer(this);
		connect(ADBCheckTimer, &QTimer::timeout, this, [=]() {
			m_androidADBDevices.clear();
			auto devices = get_adb_devices();
			for (auto iter = devices.begin(); iter != devices.end(); iter++) {
				m_androidADBDevices.insert(QString::fromStdString(*iter));
			}
		});
		ADBCheckTimer->setSingleShot(false);
		ADBCheckTimer->setInterval(1000);
		ADBCheckTimer->start();
	}

	initAndroidVids();

	auto adbDeviceCheckTimer = new QTimer(this);
	adbDeviceCheckTimer->setSingleShot(false);
	connect(adbDeviceCheckTimer, &QTimer::timeout, this, []() {});
	adbDeviceCheckTimer->start(200);

	m_eventFilter = new NativeEventFilter;
	connect(m_eventFilter, &NativeEventFilter::deviceChange, this, [=](QString devicePath, bool isAdd) {
		qDebug() << "device change, isAdd: " << isAdd << "  , path" << devicePath << "   , installing: " << m_installingDevices;

		auto onDeviceChange = [this](QString path, bool add) {
			if (add) {
				doDriverProcess(path);
			} else {
				m_androidAOADevices.remove(path);
			}
		};

		bool shouldDo = true;
		auto &pair = m_deviceEventTimestamp[devicePath];
		//如果当前设备正处在设备驱动安装阶段，那么插拔事件可能并不是用户导致的。
		//这里设置一个阈值，如果插拔之间的时间小于2s（这个值可以优化），就假定是事件是驱动安装所致，否则就是用户手动拔掉了设备
		//插入 检查定时器是否活跃，如果活跃，则停掉定时器，然后不做处理
		//移除 启动定时器，达到设置阈值后，触发设备移除操作
		if (in_processing(devicePath.toStdString().c_str())) {
			auto timerCheck = [=]() {
				if (m_deviceTimers.contains(devicePath)) {
					m_deviceTimers.value(devicePath)->stop();
					m_deviceTimers.value(devicePath)->deleteLater();
					m_deviceTimers.remove(devicePath);
				}
			};

			if (isAdd) {
				timerCheck();
			} else {
				timerCheck();
				QTimer *timer = new QTimer;
				timer->setSingleShot(true);
				timer->setInterval(2000);
				connect(timer, &QTimer::timeout, timer, [=]() {
					qDebug() << "device lost due to user unplug";
					timer->deleteLater();
					m_deviceTimers.remove(devicePath);
					kill_driver_process(devicePath.toStdString().c_str());
					onDeviceChange(devicePath, isAdd);
				});
				m_deviceTimers.insert(devicePath, timer);
				timer->start();
			}

			shouldDo = false;
		}

		//记录当前设备插拔时间
		if (isAdd)
			pair.first = QDateTime::currentMSecsSinceEpoch();
		else
			pair.second = QDateTime::currentMSecsSinceEpoch();

		if (shouldDo)
			onDeviceChange(devicePath, isAdd);
	});
	qApp->installNativeEventFilter(m_eventFilter);
}

DriverHelper::~DriverHelper()
{
	qApp->removeNativeEventFilter(m_eventFilter);
	delete m_eventFilter;

	for (auto iter = m_installs.begin(); iter != m_installs.end(); iter++) {
		QProcess *p = *iter;
		p->kill();
		p->waitForFinished();
	}
	m_installs.clear();
}

void DriverHelper::checkDevices()
{
	auto list = enumUSBDevice();
	for (auto iter = list.begin(); iter != list.end(); iter++) {
		const QString &path = iter.key();
		if (doDriverProcess(path, true))
			break;
	}
}

QSet<QString> DriverHelper::androidADBDevices()
{
	return m_androidADBDevices;
}

QSet<QString> DriverHelper::androidAOADevices()
{
	return m_androidAOADevices;
}

void DriverHelper::initAndroidVids()
{
	auto path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/vids.txt";
	QString vids;
	QFile idFile(path);
	if (idFile.exists()) {
		idFile.open(QFile::ReadOnly);
		vids = idFile.readAll();
		idFile.close();
	} else
		vids = "2d95,12d1,22d9,2717,2a70,04e8,2a45,0b05,17ef,29a9,109b,054c,18d1,19d2";

	bool ok = false;
	if (!vids.isEmpty()) {
		auto list = vids.split(',');
		for (auto it : list) {
			auto id = it.toInt(&ok, 16);
			if (ok)
				m_vids.insert(id);
		}
	}
}

QMap<QString, QString> DriverHelper::enumUSBDevice()
{
	QMap<QString, QString> result;
	HDEVINFO devInfo = SetupDiGetClassDevs(&USB_DEVICE_GUID, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (!devInfo)
		return result;

	SP_DEVINFO_DATA devData;
	devData.cbSize = sizeof(SP_DEVINFO_DATA);
	for (int i = 0;; i++) {
		SetupDiEnumDeviceInfo(devInfo, i, &devData);
		if (GetLastError() == ERROR_NO_MORE_ITEMS)
			break;

		auto getDeviceProperty = [](HDEVINFO di, SP_DEVINFO_DATA *did, DWORD property) -> QString {
			QString result;
			DWORD DataT = 0;
			LPTSTR buffer = NULL;
			DWORD buffersize = 0;
			if (!SetupDiGetDeviceRegistryProperty(di, did, property, &DataT, (PBYTE)buffer, buffersize, &buffersize))
				if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
					return result;

			buffer = (TCHAR *)LocalAlloc(LPTR, buffersize);
			if (!SetupDiGetDeviceRegistryProperty(di, did, property, &DataT, (PBYTE)buffer, buffersize, &buffersize)) {
				LocalFree(buffer);
				return result;
			}
			result = QString::fromWCharArray(buffer);
			LocalFree(buffer);
			return result;
		};
		QString deviceGUID = getDeviceProperty(devInfo, &devData, SPDRP_CLASSGUID);
		if (deviceGUID.contains(GUID_MOUSE, Qt::CaseInsensitive) || deviceGUID.contains(GUID_KEYBOARD, Qt::CaseInsensitive) ||
		    deviceGUID.contains(GUID_HID, Qt::CaseInsensitive))
			continue;

		QString deviceHardwareId = getDeviceProperty(devInfo, &devData, SPDRP_HARDWAREID);
		if (deviceHardwareId.contains("root", Qt::CaseInsensitive) || deviceHardwareId.contains("hub", Qt::CaseInsensitive))
			continue;

		qDebug() << deviceHardwareId << getDeviceProperty(devInfo, &devData, SPDRP_COMPATIBLEIDS);

		SP_DEVICE_INTERFACE_DATA interfaceData;
		interfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		if (!SetupDiEnumDeviceInterfaces(devInfo, NULL, &USB_DEVICE_GUID, i, &interfaceData)) {
			continue;
		}

		// Determine required size for interface detail data
		ULONG requiredLength = 0;
		if (!SetupDiGetDeviceInterfaceDetail(devInfo, &interfaceData, NULL, 0, &requiredLength, NULL)) {
			if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
				continue;
			}
		}

		// Allocate storage for interface detail data
		PSP_DEVICE_INTERFACE_DETAIL_DATA detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(requiredLength);
		detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		if (!SetupDiGetDeviceInterfaceDetail(devInfo, &interfaceData, detailData, requiredLength, &requiredLength, NULL)) {
			free(detailData);
			continue;
		}

		auto desc = getDeviceProperty(devInfo, &devData, SPDRP_DEVICEDESC);
		result.insert(QString::fromWCharArray(detailData->DevicePath).toLower(), desc);
		free(detailData);
	}
	SetupDiDestroyDeviceInfoList(devInfo);
	return result;
}

bool DriverHelper::doDriverProcess(QString devicePath, bool checkAoA)
{
	if (devicePath.isEmpty())
		return false;

	if (m_installingDevices.contains(devicePath) || App()->deviceInUse(devicePath)) {
		qDebug() << devicePath << " in use.";
		return false;
	}

	bool ok = false;
	auto vid = devicePath.mid(devicePath.indexOf("vid_") + 4, 4).toInt(&ok, 16);
	auto pid = devicePath.mid(devicePath.indexOf("pid_") + 4, 4).toInt(&ok, 16);

	auto androidValid = [this, checkAoA, vid, pid, devicePath]() {
		if (checkAoA && isAOADevice(vid, pid)) {
			qDebug() << "start with aoa device connected, should reconnect, device path==>" << devicePath;
			return false;
		}

		return m_vids.contains(vid);
	};

	bool isAndroid = androidValid();
	bool isApple = isAppleDevice(vid, pid);

	if (isAndroid) {
		if (!App()->mediaAvailable(PhoneType::Android))
			return false;
	} else if (isApple) {
		if (!App()->mediaAvailable(PhoneType::iOS) && !App()->mediaAvailable(PhoneType::iOSCamera))
			return false;
	} else
		return false;

	qDebug() << "using device: " << devicePath;

	m_installingDevices.insert(devicePath);

	QStringList p;
	p << QString::number(isAppleDevice(vid, pid) ? 1 : 0);
	p << QString::number(vid);
	p << QString::number(pid);
	p << devicePath;

	QProcess *process = new QProcess;
	add_process(devicePath.toStdString().c_str(), process);

	QTimer *timer = new QTimer;

	connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
		Q_UNUSED(exitStatus)

		qDebug() << (isApple ? "iOS device " : "Android device ") << "driver install result code: " << exitCode << ". install path" << devicePath;
		if (exitStatus != QProcess::NormalExit)
			qDebug() << "driver-helper crash, maybe timeout";

		m_installingDevices.remove(devicePath);

		if (timer->isActive())
			timer->stop();
		//exit code 1=>成功 2=>失败 3=>已经安装过了 4->找不到要安装的设备

		bool success = exitCode == 1 || exitCode == 3;
		if (!isApple) {
			if (success) {
				if (!isAOADevice(vid, pid)) {
					// 第一阶段, 需要发送switch指令。下一次驱动动作，依赖设备插入事件
					switchDroidToAcc(vid, pid, 1);
				} else
					m_androidAOADevices.insert(devicePath);
			} else
				qDebug() << "Android driver error, try restart";
		} else {
			if (!success)
				qDebug() << "iOS driver error, try restart";
		}

		update_driver_process(false);
		unlock_install();

		process->deleteLater();
		timer->deleteLater();
		m_installs.remove(process);

		remove_process(devicePath.toStdString().c_str());

		qDebug() << "============> driver process end; device path==> " << devicePath;
	});

	connect(timer, &QTimer::timeout, this, [=]() { process->kill(); });
	connect(process, &QProcess::errorOccurred, this, [=](QProcess::ProcessError error) { qDebug() << "fail to start driver install process: " << error; });

	timer->setSingleShot(true);
	timer->setInterval(60 * 5 * 1000);

	lock_install();
	update_driver_process(true);

	process->start("device-helper.exe", p);
	timer->start();

	return true;
}

void DriverHelper::switchDroidToAcc(int vid, int pid, int force)
{
	struct libusb_device_handle *handle = nullptr;
	unsigned char ioBuffer[2];
	int r;
	int deviceProtocol;

	libusb_context *ctx = nullptr;
	libusb_init(&ctx);
	do {
		auto handle = libusb_open_device_with_vid_pid(ctx, vid, pid);
		if (!handle) {
			qDebug("Failed to connect to device\n");
			break;
		}

		if (libusb_kernel_driver_active(handle, 0) > 0) {
			if (!force) {
				qDebug("kernel driver active, ignoring device");
				break;
			}
			if (libusb_detach_kernel_driver(handle, 0) != 0) {
				qDebug("failed to detach kernel driver, ignoring device");
				break;
			}
		}
		if (0 > (r = libusb_control_transfer(handle,
						     0xC0, //bmRequestType
						     51,   //Get Protocol
						     0, 0, ioBuffer, 2, 2000))) {
			qDebug("get protocol call failed\n");
			break;
		}

		deviceProtocol = ioBuffer[1] << 8 | ioBuffer[0];
		if (deviceProtocol < AOA_PROTOCOL_MIN || deviceProtocol > AOA_PROTOCOL_MAX) {
			qDebug("Unsupported AOA protocol %d\n", deviceProtocol);
			break;
		}

		const char *setupStrings[6];
		setupStrings[0] = vendor;
		setupStrings[1] = model;
		setupStrings[2] = description;
		setupStrings[3] = version;
		setupStrings[4] = uri;
		setupStrings[5] = serial;

		int i;
		for (i = 0; i < 6; i++) {
			if (0 >
			    (r = libusb_control_transfer(handle, 0x40, 52, 0, (uint16_t)i, (unsigned char *)setupStrings[i], strlen(setupStrings[i]), 2000))) {
				qDebug("send string %d call failed\n", i);
				break;
			}
		}

		if (deviceProtocol >= 2) {
			if (0 > (r = libusb_control_transfer(handle, 0x40, 58,
#ifdef USE_AUDIO
							     1, // 0=no audio, 1=2ch,16bit PCM, 44.1kHz
#else
							     0,
#endif
							     0, NULL, 0, 2000))) {
				qDebug("set audio call failed\n");
				break;
			}
		}

		if (0 > (r = libusb_control_transfer(handle, 0x40, 53, 0, 0, NULL, 0, 2000))) {
			qDebug("start accessory call failed\n");
			break;
		}
	} while (0);

	if (handle)
		libusb_close(handle);

	libusb_exit(ctx);
}
