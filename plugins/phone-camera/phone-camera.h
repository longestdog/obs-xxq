#pragma once

#include <QString>
#include <QDebug>
#include <qpointer.h>
#include <qfile.h>
#include <obs.h>
#include <qtcpsocket.h>
#include <qeventloop.h>
#include <qmutex.h>

#include "tcp-wrapper.h"
#include "ipc.h"
#include <util/circlebuf.h>

extern "C" {
#include "ffmpeg-decode.h"
}

struct AudioFrame {
	uint8_t *data;
	size_t data_len;
	int64_t pts;
	int serial;
};

struct VideoFrame {
	uint32_t video_info_index;
	uint8_t *data;
	size_t data_len;
	int64_t pts;
};

struct VideoInfo {
	uint8_t *data;
	size_t data_len;
};

class Decoder {
	struct ffmpeg_decode decode;

public:
	inline Decoder() { memset(&decode, 0, sizeof(decode)); }
	inline ~Decoder() { ffmpeg_decode_free(&decode); }

	inline operator ffmpeg_decode *() { return &decode; }
	inline ffmpeg_decode *operator->() { return &decode; }
};

class AVDecoder;
class PhoneCamera : public QObject {
	Q_OBJECT
public:
	PhoneCamera(obs_data_t *settings, obs_source_t *source);
	~PhoneCamera();

	PhoneType type() { return m_phoneType; }
	QMap<QString, QPair<QString, uint32_t>> deviceList(int type);
	const obs_source_t *source() const { return m_source; }

	static void ipcCallback(void *param, uint8_t *data, size_t size);
	void render(gs_effect_t *effect);

	uint32_t width() { return m_width; }
	uint32_t height() { return m_height; }

private:
	bool ipcCallbackInternal();
	bool canProcessMediaData();
	void dropAudioFrame(int64_t now);
	void dropVideoFrame(int64_t now);
	bool checkMediaPacketValid();
	void resetAudioState(bool clearAudioInfo = false);
	void resetState(PhoneType type);
	void processPendingAudioData();

	void outputAudioFrame();

	bool cacheAudioData(uint8_t *data, uint32_t size, int64_t pts, int serial);
	bool cacheVideoData(uint8_t *data, uint32_t size, int64_t pts);

	void createVideoDecoder(uint8_t *data, size_t len, bool forceRecreate, bool forceSoftware);

public slots:
	void switchPhoneType();
	void onMediaVideoInfo(const media_video_info &info);
	void onMediaAudioInfo(const media_audio_info &info);
	void onMediaData(uint8_t *data, size_t size, int64_t timestamp, bool isVideo);
	void onMediaStatus(const media_status_info &status);
	void onMediaFinish();

private:
	QFile m_videodump;

	/***************************/
	PhoneType m_phoneType = PhoneType::None;

	QMap<QString, QPair<QString, uint32_t>> m_devices;

	obs_source_t *m_source = nullptr;
	Decoder m_audioDecoder;
	media_audio_info m_audioInfo;
	Decoder m_videoDecoder;
	obs_source_frame2 frame = {0};

	uint32_t m_width{};
	uint32_t m_height{};

	TcpSocketWrapper *m_socketWrapper = nullptr;
	QString m_name;
	struct IPCServer *m_ipcServer = nullptr;
	QEventLoop m_blockEvent;

	bool m_lowLatencyMode = false;
	bool m_audioInfoReceived = false;
	bool m_videoInfoReceived = false;
	circlebuf m_mediaData;
	QList<VideoFrame> m_videoFrames;
	QList<AudioFrame> m_audioFrames;
	QMutex m_videoDataMutex;
	QMutex m_audioDataMutex;
	int64_t m_offset = LLONG_MAX;
	int64_t m_audioOffset = LLONG_MAX;
	int64_t m_extraDelay = 0;
	int64_t m_firstAudioPTS = LLONG_MAX;
	int64_t m_firstVideoPTS = LLONG_MAX;
	int64_t m_audioExtraOffset = LLONG_MAX;
	int64_t m_videoExtraOffset = LLONG_MAX;
	int64_t m_lastAudioPts = LLONG_MAX;
	int64_t m_fixAudioOffset = LLONG_MAX;
	QMutex m_ptsMutex;

	QMap<uint32_t, VideoInfo> m_videoInfos;
	uint32_t m_videoInfoIndex = 0;
	uint32_t m_lastVideoInfoIndex = 0;

	AVDecoder *m_decoder = nullptr;
	uint8_t *m_spsCache = nullptr;
	size_t m_spsCacheSize = 0;
	obs_source_frame2 m_softOutputFrame;
};
