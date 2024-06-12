#include "phone-camera.h"
#include <util/platform.h>
#include <qfile.h>
#include <qhostaddress.h>
#include <qdebug.h>
#include <qtimer.h>
#include <qjsonarray.h>
#include <quuid.h>
#include <qprocess.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include "tool.h"
#include "video-decoder.h"

#define PHONE_DEVICE_TYPE "type"
#define DEVICE_ID "device_id"
#define LOW_LATENCY_MODE "lowLatencyMode"
#define CONNECT_TIMEOUT 2000

static int PhoneCameraInstanceCount = 0;
static QProcess BackendProcess;
static QString BackendProcessExeName = "mobile-media-backend.exe";
static QString BackendProcessPath = QString("\"%1/%2\"").arg(QApplication::applicationDirPath()).arg(BackendProcessExeName);

//#define DEBUG_MEDIA_BACKEND

//#define DUMP_VIDEO
PhoneCamera::PhoneCamera(obs_data_t *settings, obs_source_t *source) : m_source(source)
{
	m_lowLatencyMode = obs_data_get_bool(settings, "lowLatencyMode");
#ifdef DUMP_VIDEO
	m_videodump.setFileName("dump.h264");
	m_videodump.open(QFile::ReadWrite);
#endif // DUMP_VIDEO

	auto socket = new QTcpSocket(this);
	m_socketWrapper = new TcpSocketWrapper(socket);
	connect(m_socketWrapper, &TcpSocketWrapper::msgReceived, this, [=](QJsonObject msg) {
		int type = msg["type"].toInt();
		if ((MsgType)type == MsgType::DeviceListResult) {
			QJsonArray array = msg["data"].toArray();
			for (int i = 0; i < array.size(); i++) {
				auto obj = array.at(i).toObject();
				m_devices.insert(obj["id"].toString(), {obj["name"].toString(), obj["handle"].toString().toULong()});
			}
			m_blockEvent.quit();
		}
	});

	socket->connectToHost(QHostAddress::LocalHost, 51338);
	bool ret = socket->waitForConnected(CONNECT_TIMEOUT);

	circlebuf_init(&m_mediaData);
	m_name = QUuid::createUuid().toString(QUuid::WithoutBraces);
	ipc_server_create(&m_ipcServer, m_name.toUtf8().data(), PhoneCamera::ipcCallback, this);

	blog(LOG_INFO, "%s->command socket connect result: %s", m_name.toStdString().c_str(), (ret ? "success" : "fail"));
}

PhoneCamera::~PhoneCamera()
{
	ipc_server_destroy(&m_ipcServer);
	circlebuf_free(&m_mediaData);
	if (m_spsCache)
		bfree(m_spsCache);

	if (m_decoder)
		delete m_decoder;

	while (m_audioFrames.size() > 0) {
		bfree(m_audioFrames.front().data);
		m_audioFrames.pop_front();
	}
	while (m_videoFrames.size() > 0) {
		bfree(m_videoFrames.front().data);
		m_videoFrames.pop_front();
	}
	for (auto iter = m_videoInfos.begin(); iter != m_videoInfos.end(); iter++) {
		VideoInfo &f = iter.value();
		bfree(f.data);
	}

	clearStatusTexture(true);
#ifdef DUMP_VIDEO
	m_videodump.close();
#endif
}

void PhoneCamera::clearStatusTexture(bool lock)
{
	if (lock)
		obs_enter_graphics();

	if (m_statusTexture) {
		gs_texture_destroy(m_statusTexture);
		m_statusTexture = nullptr;
	}

	if (lock)
		obs_leave_graphics();
}

void PhoneCamera::ipcCallback(void *param, uint8_t *data, size_t size)
{
	PhoneCamera *p = (PhoneCamera *)param;
	circlebuf_push_back(&p->m_mediaData, data, size);
	while (true) {
		if (p->ipcCallbackInternal())
			continue;
		else
			break;
	}
}

bool PhoneCamera::canProcessMediaData()
{
	bool ret = false;
	m_ptsMutex.lock();
	bool timeOffsetInited = (m_videoExtraOffset != LLONG_MAX || m_audioExtraOffset != LLONG_MAX);
	if (!timeOffsetInited) {
		if (m_firstAudioPTS != LLONG_MAX && m_firstVideoPTS != LLONG_MAX) {
			if (m_firstVideoPTS > m_firstAudioPTS) {
				m_videoExtraOffset = 0;
				m_audioExtraOffset = m_firstAudioPTS - m_firstVideoPTS;
			} else {
				m_videoExtraOffset = m_firstVideoPTS - m_firstAudioPTS;
				m_audioExtraOffset = 0;
			}
			ret = true;
		}
	} else
		ret = true;
	m_ptsMutex.unlock();

	return ret;
}

void PhoneCamera::dropAudioFrame(int64_t now)
{
	while (m_audioFrames.size() >= 2) {
		auto iter = m_audioFrames.begin();
		auto p1 = iter->pts;

		auto next = ++iter;
		auto p2 = next->pts;

		auto p1ts = p1 + m_audioOffset + m_extraDelay;
		auto p2ts = p2 + m_audioOffset + m_extraDelay;

		if (p1ts < now && p2ts < now && now - p2ts > 150) {
			bfree(m_audioFrames.front().data);
			m_audioFrames.pop_front();
		} else
			break;
	}
}

void PhoneCamera::dropVideoFrame(int64_t now)
{
	while (m_videoFrames.size() >= 2) {
		auto iter = m_videoFrames.begin();
		auto p1 = iter->pts;
		auto p1ts = iter->pts + m_offset + m_extraDelay;

		auto next = ++iter;
		auto p2 = next->pts;
		auto p2ts = next->pts + m_offset + m_extraDelay;

		VideoFrame &frame = m_videoFrames.front();
		if (frame.video_info_index != m_lastVideoInfoIndex)
			break;

		if ((p1 == 0 && p2 == 0) || p1ts < now && p2ts < now && now - p2ts > 60) {
			if (m_decoder) {
				int ret = m_decoder->send(frame.data, frame.data_len);
				while (ret >= 0) {
					ret = m_decoder->recv(true);
				}
			}
			bfree(frame.data);
			m_videoFrames.pop_front();
		} else
			break;
	}
}

void PhoneCamera::outputAudioFrame()
{
	if (m_audioFrames.size() <= 0 || !m_audioInfo.samples_per_sec || m_audioInfo.format == AUDIO_FORMAT_UNKNOWN || m_audioInfo.speakers == SPEAKERS_UNKNOWN)
		return;
	AudioFrame &frame = m_audioFrames.front();
	obs_source_audio audio;
	audio.format = m_audioInfo.format;
	audio.samples_per_sec = m_audioInfo.samples_per_sec;
	audio.speakers = m_audioInfo.speakers;
	audio.frames = (uint32_t)frame.data_len / (m_audioInfo.speakers * sizeof(short));
	audio.timestamp = os_gettime_ns();
	audio.data[0] = frame.data;
	obs_source_output_audio(m_source, &audio);

	bfree(frame.data);
	m_audioFrames.removeFirst();
}

void PhoneCamera::processPendingAudioData()
{
	while (true) {
		if (!canProcessMediaData() || m_audioFrames.size() <= 0)
			break;

		int64_t pts = m_audioFrames.front().pts;
		if (pts <= 0)
			outputAudioFrame();
		else {
			int64_t now_ms = (int64_t)os_gettime_ns() / 1000000;

			if (m_audioOffset == LLONG_MAX) {
				m_audioOffset = now_ms - pts + m_audioExtraOffset;
			}

			dropAudioFrame(now_ms);

			int64_t target_pts = m_audioFrames.front().pts + m_audioOffset + m_extraDelay;
			if (target_pts <= now_ms) {
				outputAudioFrame();
			} else
				break;
		}
	}
}

bool PhoneCamera::checkMediaPacketValid()
{
	static size_t maxPacketSize = 1024 * 1024 * 10;

	size_t headerSize = sizeof(av_packet_info);
	if (m_mediaData.size < headerSize)
		return false;

	av_packet_info headerInfo = {0};
	circlebuf_peek_front(&m_mediaData, &headerInfo, headerSize);

	size_t reqSize = headerInfo.size;
	if (m_mediaData.size < reqSize + headerSize)
		return false;

	size_t requiredSize = 0;
	switch (headerInfo.type) {
	case av_packet_type::FFM_MIRROR_STATUS:
		requiredSize = sizeof(media_status_info);
		break;
	case av_packet_type::FFM_MEDIA_VIDEO_INFO:
		requiredSize = sizeof(media_video_info);
		break;
	case av_packet_type::FFM_MEDIA_AUDIO_INFO:
		requiredSize = sizeof(media_audio_info);
		break;
	default:
		requiredSize = maxPacketSize;
		break;
	}

	bool valid = true;
	if (headerInfo.type <= av_packet_type::FFM_PACKET_AUDIO) {
		valid = reqSize <= requiredSize;
	} else
		valid = reqSize == requiredSize;

	if (!valid)
		circlebuf_free(&m_mediaData);

	return valid;
}

void PhoneCamera::resetAudioState(bool clearAudioInfo)
{
	m_audioOffset = LLONG_MAX;
	for (auto iter = m_audioFrames.begin(); iter != m_audioFrames.end(); iter++) {
		AudioFrame &f = *iter;
		bfree(f.data);
	}
	m_audioFrames.clear();
	if (clearAudioInfo)
		memset(&m_audioInfo, 0, sizeof(media_audio_info));
	m_lastAudioPts = LLONG_MAX;
	m_fixAudioOffset = LLONG_MAX;
}

void PhoneCamera::updateSyncOffset()
{
	int ret = 0;

	if (!m_lowLatencyMode) {
		if (m_phoneType == PhoneType::iOSAirplay)
			ret = 500;
		else if (m_phoneType == PhoneType::AndroidRtmp)
			ret = 300;
		else if (m_phoneType == PhoneType::iOSRtmp)
			ret = 400;
	}

	m_extraDelay = ret;
}

void PhoneCamera::resetState(PhoneType type)
{
	m_phoneType = type;
	updateSyncOffset();
	m_videoDataMutex.lock();
	m_videoInfoReceived = false;
	m_offset = LLONG_MAX;
	for (auto iter = m_videoFrames.begin(); iter != m_videoFrames.end(); iter++) {
		VideoFrame &f = *iter;
		bfree(f.data);
	}
	m_videoFrames.clear();
	m_videoInfoIndex = 0;
	m_lastVideoInfoIndex = 0;
	for (auto iter = m_videoInfos.begin(); iter != m_videoInfos.end(); iter++) {
		VideoInfo &f = iter.value();
		bfree(f.data);
	}
	m_videoInfos.clear();
	m_videoDataMutex.unlock();

	m_audioInfoReceived = false;
	resetAudioState(true);

	m_ptsMutex.lock();
	if (type == PhoneType::iOSRtmp || type == PhoneType::AndroidRtmp)
		m_audioExtraOffset = LLONG_MAX;
	else if (type == PhoneType::iOSAirplay)
		m_audioExtraOffset = -100;
	else
		m_audioExtraOffset = 0;

	m_firstAudioPTS = LLONG_MAX;
	m_firstVideoPTS = LLONG_MAX;

	if (type == PhoneType::iOSRtmp || type == PhoneType::AndroidRtmp)
		m_videoExtraOffset = LLONG_MAX;
	else
		m_videoExtraOffset = 0;
	m_ptsMutex.unlock();
}

bool PhoneCamera::cacheAudioData(uint8_t *data, uint32_t size, int64_t pts, int serial)
{
	if (!m_audioInfoReceived)
		return false;

	pts = pts / 1000000;

	m_ptsMutex.lock();
	if (m_firstAudioPTS == LLONG_MAX)
		m_firstAudioPTS = pts;
	m_ptsMutex.unlock();

	//这边的pts修正逻辑只有ios镜像投屏才会走到
	if (m_lastAudioPts == LLONG_MAX) {
		m_lastAudioPts = pts;
	} else {
		if (m_fixAudioOffset != LLONG_MAX) {
			pts = pts - m_fixAudioOffset;
		} else {
			int64_t frameDuration = size * 1000 / (m_audioInfo.samples_per_sec * 4); //发过来的pcm都是16bit 双声道
			if (pts - m_lastAudioPts > 3600 * 24) {                                  //如果相邻时间戳的值相差太大，需要修正
				m_fixAudioOffset = pts - m_lastAudioPts - frameDuration;
				pts = pts - m_fixAudioOffset;
			}
		}
	}
	m_audioFrames.push_back({data, size, pts, serial});
	m_lastAudioPts = pts;

	return true;
}
bool PhoneCamera::cacheVideoData(uint8_t *data, uint32_t size, int64_t pts)
{
	if (!m_videoInfoReceived)
		return false;

	QMetaObject::invokeMethod(
		this, [this]() { clearStatusTexture(true); }, Qt::QueuedConnection);

	pts = pts / 1000000;

	m_ptsMutex.lock();
	if (m_firstVideoPTS == LLONG_MAX)
		m_firstVideoPTS = pts;
	m_ptsMutex.unlock();

	m_videoDataMutex.lock();
	m_videoFrames.push_back({m_videoInfoIndex, data, size, pts});
	m_videoDataMutex.unlock();

	return true;
}

bool PhoneCamera::ipcCallbackInternal()
{
	processPendingAudioData();
	if (!checkMediaPacketValid())
		return false;

	av_packet_info headerInfo = {};
	circlebuf_pop_front(&m_mediaData, &headerInfo, sizeof(av_packet_info)); // remove it
	switch (headerInfo.type) {
	case av_packet_type::FFM_MIRROR_STATUS: {
		media_status_info status;
		circlebuf_pop_front(&m_mediaData, &status, sizeof(media_status_info));
		onMediaStatus(status);
	} break;
	case av_packet_type::FFM_MEDIA_AUDIO_INFO: {
		media_audio_info info;
		circlebuf_pop_front(&m_mediaData, &info, sizeof(media_audio_info));
		onMediaAudioInfo(info);
	} break;
	case av_packet_type::FFM_MEDIA_VIDEO_INFO: {
		media_video_info info;
		circlebuf_pop_front(&m_mediaData, &info, sizeof(media_video_info));
		onMediaVideoInfo(info);
	} break;
	case av_packet_type::FFM_PACKET_AUDIO:
	case av_packet_type::FFM_PACKET_VIDEO: {
		uint8_t *popBuffer = (uint8_t *)bzalloc(headerInfo.size);
		circlebuf_pop_front(&m_mediaData, popBuffer, headerInfo.size);
		bool cached = headerInfo.type == av_packet_type::FFM_PACKET_AUDIO
				      ? cacheAudioData(popBuffer, headerInfo.size, headerInfo.pts, headerInfo.serial)
				      : cacheVideoData(popBuffer, headerInfo.size, headerInfo.pts);
		if (!cached)
			bfree(popBuffer);
	} break;
	default:
		break;
	}

	return true;
}

void PhoneCamera::onMediaVideoInfo(const media_video_info &info)
{
	if (info.video_extra_len == 0)
		return;

	m_videoDataMutex.lock();
	m_videoInfoReceived = true;
	auto cache = (uint8_t *)bzalloc(info.video_extra_len);
	memcpy(cache, info.video_extra, info.video_extra_len);
	m_videoInfoIndex++;
	VideoInfo vi;
	vi.data = cache;
	vi.data_len = info.video_extra_len;
	m_videoInfos.insert(m_videoInfoIndex, std::move(vi));
	m_videoDataMutex.unlock();
}

void PhoneCamera::onMediaAudioInfo(const media_audio_info &info)
{
	m_audioInfoReceived = true;
	m_audioInfo = info;
}

void PhoneCamera::onMediaStatus(const media_status_info &status)
{
	auto updateStatusImage = [this](const media_status_info &status) {
		QMetaObject::invokeMethod(
			this,
			[this, status]() {
				obs_enter_graphics();
				clearStatusTexture(false);
				m_statusTexture = prepareStateTexture(status.type, status.status);
				obs_leave_graphics();
			},
			Qt::QueuedConnection);
	};
	switch (status.status) {
	case media_status::Media_START:
		resetState(status.type);
		updateStatusImage(status);
		break;
	case media_status::Media_INIT:
		QMetaObject::invokeMethod(
			this,
			[this, status]() {
				obs_source_set_monitoring_type(m_source, (isAndroidAOAPhoneType(status.type) || isRtmpPhoneType(status.type))
										 ? OBS_MONITORING_TYPE_NONE
										 : OBS_MONITORING_TYPE_MONITOR_ONLY);
			},
			Qt::QueuedConnection);
		updateStatusImage(status);
		break;
	case media_status::Media_STOP:
	case media_status::Media_DEVICE_LOST:
		updateStatusImage(status);
		break;
	case media_status::Media_AUDIO_SESSION_START:
		resetAudioState();
		break;
	default:
		break;
	}

	blog(LOG_INFO, "onMediaStatus: %d", (int)status.status);
}

void PhoneCamera::createVideoDecoder(uint8_t *data, size_t len, bool forceRecreate, bool forceSoftware)
{
	if (!forceRecreate && (m_decoder && !m_decoder->spsChanged(data, len)))
		return;
	if (m_decoder)
		delete m_decoder;

	auto decoder = new AVDecoder;
	bool success = decoder->init(data, len, forceSoftware);
	if (!success) {
		delete decoder;
		blog(LOG_ERROR, "fail to create video decoder for PhoneCamera");
		return;
	}
	m_decoder = decoder;
	blog(LOG_INFO, "mirror decoder init complete, use hardware: %s", m_decoder->isHWDecode() ? "true" : "false");
}

void PhoneCamera::render(gs_effect_t *effect)
{
	if (m_statusTexture) {
		m_width = gs_texture_get_width(m_statusTexture);
		m_height = gs_texture_get_height(m_statusTexture);
		gs_effect_set_texture(gs_effect_get_param_by_name(effect, "image"), m_statusTexture);
		gs_draw_sprite(m_statusTexture, 0, m_width, m_height);
		return;
	}

	m_videoDataMutex.lock();
	while (canProcessMediaData() && m_videoFrames.size() > 0) {
		int64_t now_ms = (int64_t)os_gettime_ns() / 1000000;
		int64_t target_pts = 0;

		if (m_offset == LLONG_MAX) {
			VideoFrame &frame = m_videoFrames.front();
			m_offset = now_ms - frame.pts + m_videoExtraOffset;
		}

		dropVideoFrame(now_ms);

		VideoFrame &framev = m_videoFrames.front();
		target_pts = framev.pts + m_offset + m_extraDelay;
		if (target_pts <= now_ms + 2 || framev.pts == 0) { //+2 为了pts的误差，视频帧时间戳超过可播放时间2ms，也认为当前是可以播放的。
			if (framev.video_info_index != m_lastVideoInfoIndex) {
				const VideoInfo &vi = m_videoInfos[framev.video_info_index];
				if (!m_spsCache) {
					m_spsCache = (uint8_t *)bzalloc(vi.data_len);
					m_spsCacheSize = vi.data_len;
				} else if (m_spsCacheSize < vi.data_len) {
					m_spsCache = (uint8_t *)brealloc(m_spsCache, vi.data_len);
					m_spsCacheSize = vi.data_len;
				}

				memcpy(m_spsCache, vi.data, vi.data_len);

				createVideoDecoder(vi.data, vi.data_len, false, false);
				bfree(vi.data);
				m_videoInfos.remove(framev.video_info_index);
				m_lastVideoInfoIndex = framev.video_info_index;
			}

			if (m_decoder) {
				int ret = m_decoder->send(framev.data, framev.data_len);
				if (ret == AVERROR_INVALIDDATA) {
					createVideoDecoder(m_spsCache, m_spsCacheSize, true, true);
					ret = m_decoder->send(framev.data, framev.data_len);
				}
				while (ret >= 0) {
					ret = m_decoder->recv(false);
				}
			}

			bfree(framev.data);
			m_videoFrames.pop_front();
		}
		break;
	}
	m_videoDataMutex.unlock();

	if (!m_decoder)
		return;

	m_width = m_decoder->width();
	m_height = m_decoder->height();
	if (m_decoder->isHWDecode()) {
		auto tex = m_decoder->texture();
		if (tex) {
			gs_effect_set_texture(gs_effect_get_param_by_name(effect, "image"), tex);
			gs_draw_sprite(tex, 0, gs_texture_get_width(tex), gs_texture_get_height(tex));
		}
	} else {
		obs_source_set_videoframe(m_source, m_decoder->frame());
		obs_source_draw_videoframe(m_source);
	}
}

void PhoneCamera::switchPhoneType(int type, QString deviceId)
{
	if (type == (int)PhoneType::iOSRtmp || type == (int)PhoneType::AndroidRtmp) {
		auto pid = findProcessListeningToPort(1935);
		if (pid != 0 && pid != BackendProcess.processId()) {
			blog(LOG_INFO, "rtmp port used, cannot listen to it");
			QMessageBox *msgBox = new QMessageBox(QApplication::activeModalWidget());
			msgBox->setWindowFlag(Qt::WindowStaysOnTopHint);
			msgBox->setIcon(QMessageBox::Information);
			msgBox->setText(u8"无线投屏服务启动失败\n请关闭其他投屏服务软件后重启助手");
			msgBox->addButton(u8"确定", QMessageBox::RejectRole);
			msgBox->setAttribute(Qt::WA_DeleteOnClose); // delete pointer after close
			msgBox->setModal(false);
			msgBox->show();
			return;
		}
	}
	QJsonObject req;
	req["type"] = (int)MsgType::MediaTask;
	QJsonObject data;
	data["name"] = m_name;
	data["deviceType"] = type;
	data["deviceId"] = deviceId;
	req["data"] = data;

	m_socketWrapper->sendMsg(QJsonDocument(req));
}

void PhoneCamera::updateLatencyMode(bool lowLatency)
{
	if (m_lowLatencyMode ^ lowLatency) {
		m_lowLatencyMode = lowLatency;
		updateSyncOffset();
	}
}

QMap<QString, QPair<QString, uint32_t>> PhoneCamera::deviceList(int type)
{
	m_devices.clear();

	if (type != (int)PhoneType::iOSAirplay && type != (int)PhoneType::iOSRtmp && type != (int)PhoneType::AndroidRtmp) {
		QJsonObject req;
		if (type == (int)PhoneType::iOS)
			req["type"] = (int)MsgType::iOSDeviceList;
		else if (isAndroidAOAPhoneType((PhoneType)type))
			req["type"] = (int)MsgType::AndroidDeviceList;
		else if (type == (int)PhoneType::AndroidADB)
			req["type"] = (int)MsgType::AndroidADBDeviceList;

		QTimer timer;
		timer.setSingleShot(true);
		connect(&timer, &QTimer::timeout, &m_blockEvent, &QEventLoop::quit);
		m_socketWrapper->sendMsg(QJsonDocument(req));
		timer.start(100);
		m_blockEvent.exec(QEventLoop::ExcludeUserInputEvents);
	}

	return m_devices;
}

static const char *GetPhoneMirrorInputName(void *)
{
	return "xxq phone mirror plugin";
}

static const char *GetPhoneCameraInputName(void *)
{
	return "xxq phone camera plugin";
}

static void startBackendProcess()
{
#ifdef DEBUG_MEDIA_BACKEND
	return;
#endif
	if (PhoneCameraInstanceCount == 0) {
		BackendProcess.start(BackendProcessPath);
		BackendProcess.waitForStarted();
		blog(LOG_INFO, "mobile-media-backend.exe started.");
	}
	PhoneCameraInstanceCount++;
}

static void stopBackendProcess()
{
#ifdef DEBUG_MEDIA_BACKEND
	return;
#endif
	PhoneCameraInstanceCount--;
	if (PhoneCameraInstanceCount <= 0) {
		PhoneCameraInstanceCount = 0;
		BackendProcess.kill();
		blog(LOG_INFO, "mobile-media-backend.exe stopped.");
	}
}

static void UpdatePhoneCameraInput(void *data, obs_data_t *settings);
static void *CreatePhoneCameraInput(obs_data_t *settings, obs_source_t *source)
{
	startBackendProcess();

	PhoneCamera *cameraInput = nullptr;

	try {
		cameraInput = new PhoneCamera(settings, source);
		UpdatePhoneCameraInput(cameraInput, settings);
	} catch (const char *error) {
		blog(LOG_ERROR, "Could not create device '%s': %s", obs_source_get_name(source), error);
	}

	return cameraInput;
}

static void DestroyPhoneCameraInput(void *data)
{
	auto p = reinterpret_cast<PhoneCamera *>(data);
	delete p;

	stopBackendProcess();
}

static void PhoneCameraRender(void *data, gs_effect_t *effect)
{
	auto p = reinterpret_cast<PhoneCamera *>(data);
	p->render(effect);
}

static uint32_t GetPhoneCameraWidth(void *data)
{
	auto *s = (PhoneCamera *)data;
	return s->width();
}

static uint32_t GetPhoneCameraHeight(void *data)
{
	auto *s = (PhoneCamera *)data;
	return s->height();
}

static void DeactivatePhoneCameraInput(void *data)
{
	auto cameraInput = reinterpret_cast<PhoneCamera *>(data);
	//cameraInput->deactivate();
}

static void ActivatePhoneCameraInput(void *data)
{
	auto cameraInput = reinterpret_cast<PhoneCamera *>(data);
	//cameraInput->activate();
}

static bool UpdateClicked(obs_properties_t *props, obs_property_t *p, void *data)
{
	PhoneCamera *camera = reinterpret_cast<PhoneCamera *>(data);
	auto settings = obs_source_get_settings(camera->source());
	obs_data_release(settings);
	auto devices = camera->deviceList(obs_data_get_int(settings, PHONE_DEVICE_TYPE));

	obs_property_t *dev_list = obs_properties_get(props, DEVICE_ID);
	obs_property_list_clear(dev_list);

	obs_property_list_add_string(dev_list, "auto", "auto");
	for (auto iter = devices.begin(); iter != devices.end(); iter++) {
		obs_property_list_add_string(dev_list, iter.value().first.toUtf8().data(), iter.key().toUtf8().data());
	}

	return true;
}

static bool TypeSelectionChanged(obs_properties_t *props, obs_property_t *p, obs_data_t *settings)
{
	auto camera = obs_properties_get_param(props);
	UpdateClicked(props, p, camera);
	return true;
}

static obs_properties_t *GetPhoneCameraProperties(void *data)
{
	UNUSED_PARAMETER(data);

	PhoneCamera *camera = reinterpret_cast<PhoneCamera *>(data);
	obs_properties_t *ppts = obs_properties_create();
	obs_properties_set_param(ppts, camera, NULL);

	obs_properties_add_bool(ppts, LOW_LATENCY_MODE, "enable low latency mode");

	obs_property_t *pd = obs_properties_add_list(ppts, PHONE_DEVICE_TYPE, "device type", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(pd, "iOS", (int)PhoneType::iOS);
	obs_property_list_add_int(pd, "iOS-Camera", (int)PhoneType::iOSCamera);
	obs_property_list_add_int(pd, "Android-Wired-Screen", (int)PhoneType::AndroidAOAScreen);
	obs_property_list_add_int(pd, "Android-Wired-Camera", (int)PhoneType::AndroidAOACamera);
	obs_property_list_add_int(pd, "Android-ADB", (int)PhoneType::AndroidADB);
	obs_property_list_add_int(pd, "iOS-Airplay", (int)PhoneType::iOSAirplay);
	obs_property_list_add_int(pd, "iOS-Rtmp", (int)PhoneType::iOSRtmp);
	obs_property_list_add_int(pd, "Android-Rtmp", (int)PhoneType::AndroidRtmp);
	obs_property_set_modified_callback(pd, TypeSelectionChanged);

	obs_property_t *p = obs_properties_add_list(ppts, DEVICE_ID, "iOS device list", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	UpdateClicked(ppts, p, camera);

	obs_properties_add_button(ppts, "update", "update device list", UpdateClicked);

	return ppts;
}

static void GetPhoneCameraDefaults(obs_data_t *settings)
{
	obs_data_set_default_int(settings, PHONE_DEVICE_TYPE, (int)PhoneType::iOS);
	obs_data_set_default_string(settings, DEVICE_ID, "auto");
	obs_data_set_default_bool(settings, LOW_LATENCY_MODE, false);
}

static void SavePhoneCameraInput(void *data, obs_data_t *settings)
{
	UNUSED_PARAMETER(data);
	UNUSED_PARAMETER(settings);
}

static void UpdatePhoneCameraInput(void *data, obs_data_t *settings)
{
	PhoneCamera *input = reinterpret_cast<PhoneCamera *>(data);

	PhoneType type = (PhoneType)obs_data_get_int(settings, PHONE_DEVICE_TYPE);
	QString deviceId = obs_data_get_string(settings, DEVICE_ID);
	QMetaObject::invokeMethod(input, "switchPhoneType", Q_ARG(int, (int)type), Q_ARG(QString, deviceId));

	QMetaObject::invokeMethod(input, "updateLatencyMode", Q_ARG(bool, obs_data_get_bool(settings, LOW_LATENCY_MODE)));
}

static void OnPhoneCameraInputCommand(void *data, obs_data_t *command)
{
	PhoneCamera *input = reinterpret_cast<PhoneCamera *>(data);
	const char *cmdType = obs_data_get_string(command, "type");
	if (strcmp("changeBackend", cmdType) == 0) {
		int t = obs_data_get_int(command, "backendType");
		obs_data_set_int(command, PHONE_DEVICE_TYPE, t);
		obs_source_update(input->source(), command);
	}
}

void RegisterPhoneCamera()
{
#ifndef DEBUG_MEDIA_BACKEND
	os_kill_process(BackendProcessExeName.toUtf8().data());
#endif // !

	obs_source_info info = {};
	info.id = "win_airplay";
	info.type = OBS_SOURCE_TYPE_INPUT;
	info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_AUDIO | OBS_SOURCE_DO_NOT_DUPLICATE;
	info.get_name = GetPhoneMirrorInputName;

	info.create = CreatePhoneCameraInput;
	info.destroy = DestroyPhoneCameraInput;

	info.video_render = PhoneCameraRender;
	info.get_width = GetPhoneCameraWidth;
	info.get_height = GetPhoneCameraHeight;

	info.deactivate = DeactivatePhoneCameraInput;
	info.activate = ActivatePhoneCameraInput;

	info.get_defaults = GetPhoneCameraDefaults;
	info.get_properties = GetPhoneCameraProperties;
	info.save = SavePhoneCameraInput;
	info.update = UpdatePhoneCameraInput;
	info.make_command = OnPhoneCameraInputCommand;
	obs_register_source(&info);

	info.id = "phone_camera";
	info.get_name = GetPhoneCameraInputName;
	obs_register_source(&info);
}
