#include "media-source.h"
#include <qtcpsocket.h>
#include <qhostaddress.h>
#include <qdebug.h>
#include <media-io/audio-io.h>
#include "wired-media-task.h"
#include "airplay-media-task.h"
#include "rtmp-media-task.h"

#include "application.h"

MediaSource::MediaSource(QObject *parent) : QObject(parent)
{
	qDebug() << "MediaSource created";
}

MediaSource::~MediaSource()
{
	App()->mediaObjectRegister(m_phoneType, this, false);

	if (m_mediaTask)
		delete m_mediaTask;

	ipc_client_destroy(&m_ipcClient);
	qDebug() << "MediaSource destroyed.";
}

void MediaSource::connectToMediaTarget(QString name)
{
	ipc_client_create(&m_ipcClient, name.toUtf8().data());
}
extern "C" {
extern int64_t os_gettime_ns();
}
void MediaSource::setCurrentDevice(PhoneType type, QString deviceId)
{
	if (m_phoneType == type && m_device == deviceId)
		return;

	qDebug() << "MediaSource setCurrentDevice==> " << (int)type << deviceId;
	if (type != m_phoneType && m_mediaTask) {
		m_mediaTask->deleteLater();
		m_mediaTask = nullptr;
	}

	App()->mediaObjectRegister(m_phoneType, this, false); //去掉之前注册的，如果有的话
	App()->mediaObjectRegister(type, this, true);

	m_phoneType = (PhoneType)type;
	m_device = deviceId;
	if (!m_mediaTask) {
		if (m_phoneType == PhoneType::iOSAirplay)
			m_mediaTask = new AirplayMediaTask();
		else if (m_phoneType == PhoneType::iOSRtmp || m_phoneType == PhoneType::AndroidRtmp)
			m_mediaTask = new RtmpMediaTask(m_phoneType);
		else
			m_mediaTask = new WiredMediaTask(m_phoneType);
		connect(
			m_mediaTask, &BaseMediaTask::mediaData, this,
			[=](char *data, int size, int64_t timestamp, bool isVideo) {
				int res = 0;
				if (isVideo) {

					av_packet_info info;
					info.type = timestamp == ((int64_t)UINT64_C(0x8000000000000000)) ? av_packet_type::FFM_MEDIA_VIDEO_INFO
													 : av_packet_type::FFM_PACKET_VIDEO;
					info.pts = timestamp;
					if (info.type == av_packet_type::FFM_MEDIA_VIDEO_INFO) {
						info.size = sizeof(media_video_info);
						media_video_info video_info;
						memcpy(video_info.video_extra, data, size);
						video_info.video_extra_len = size;
						res = ipc_client_write_2(m_ipcClient, &info, sizeof(av_packet_info), &video_info, sizeof(media_video_info), 0);
					} else {
						info.size = size;
						res = ipc_client_write_2(m_ipcClient, &info, sizeof(av_packet_info), data, size, 0);
					}
				} else {
					if (timestamp == ((int64_t)UINT64_C(0x8000000000000000))) {
						uint32_t *ud = (uint32_t *)data;
						media_audio_info audio_info;
						audio_info.format = (enum audio_format)ud[0];
						audio_info.samples_per_sec = ud[1];
						audio_info.speakers = (enum speaker_layout)ud[2];

						av_packet_info info;
						info.type = av_packet_type::FFM_MEDIA_AUDIO_INFO;
						info.pts = timestamp;
						info.size = sizeof(media_audio_info);
						res = ipc_client_write_2(m_ipcClient, &info, sizeof(av_packet_info), &audio_info, sizeof(media_audio_info), 0);

					} else {
						av_packet_info info;
						info.type = av_packet_type::FFM_PACKET_AUDIO;
						info.pts = timestamp;
						info.size = size;
						res = ipc_client_write_2(m_ipcClient, &info, sizeof(av_packet_info), data, size, 0);
					}
				}
				if (res == -1 && !m_emitError) {
					m_emitError = true;
					QMetaObject::invokeMethod(this, "mediaTargetLost");
				}
			},
			Qt::DirectConnection);
		connect(
			m_mediaTask, &BaseMediaTask::mediaState, this,
			[=](media_status status) {
				media_status_info statusInfo;
				statusInfo.status = status;
				statusInfo.type = m_phoneType;
				av_packet_info info;
				info.type = av_packet_type::FFM_MIRROR_STATUS;
				info.size = sizeof(media_status_info);
				info.pts = 0;
				int res = ipc_client_write_2(m_ipcClient, &info, sizeof(av_packet_info), &statusInfo, sizeof(media_status_info), 0);
				if (res == -1 && !m_emitError) {
					m_emitError = true;
					QMetaObject::invokeMethod(this, "mediaTargetLost");
				}
			},
			Qt::DirectConnection);

		emit m_mediaTask->mediaState(media_status::Media_INIT);
	}
	m_mediaTask->setExpectedDevice(deviceId);
	App()->driverHelper().checkDevices();
}

void MediaSource::mediaTargetLost()
{
	qDebug() << "MediaSource target lost.";
	deleteLater();
}

PhoneType MediaSource::sourceType()
{
	return m_phoneType;
}
