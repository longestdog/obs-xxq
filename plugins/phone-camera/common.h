#pragma once

enum class av_packet_type {
	FFM_PACKET_VIDEO,
	FFM_PACKET_AUDIO,
	FFM_MEDIA_VIDEO_INFO,
	FFM_MEDIA_AUDIO_INFO,
	FFM_MIRROR_STATUS,
};

enum class media_status { // same as obs_source_mirror_status
	Media_ERROR,
	Media_DEVICE_CONNECTED,
	Media_DEVICE_LOST,
	Media_INIT,
	Media_START,
	Media_STOP,
	Media_OUTPUT,
	Media_AUDIO_SESSION_START,
};

#define FFM_SUCCESS 0
#define FFM_ERROR -1
#define FFM_UNSUPPORTED -2

struct av_packet_info {
	int64_t pts;
	uint32_t size;
	int serial;
	enum av_packet_type type;
};

struct media_audio_info {
	enum speaker_layout speakers;
	enum audio_format format;
	uint32_t samples_per_sec;
};

struct media_video_info {
	uint8_t video_extra[10240];
	size_t video_extra_len;
};

enum class PhoneType {
	None,
	iOS,
	AndroidAOAScreen,
	AndroidAOACamera,
	AndroidADB,
	iOSCamera,
	iOSAirplay,
	iOSRtmp,
	AndroidRtmp,
};

static inline bool isWirelessPhoneType(PhoneType type)
{
	return type == PhoneType::AndroidRtmp || type == PhoneType::iOSRtmp || type == PhoneType::iOSAirplay;
}

static inline bool isAndroidAOAPhoneType(PhoneType type)
{
	return type == PhoneType::AndroidAOACamera || type == PhoneType::AndroidAOAScreen;
}

static inline bool isAndroidWiredPhoneType(PhoneType type)
{
	return isAndroidAOAPhoneType(type) || type == PhoneType::AndroidADB;
}

static inline bool isiOSWiredPhoneType(PhoneType type)
{
	return type == PhoneType::iOS || type == PhoneType::iOSCamera;
}

static inline bool isRtmpPhoneType(PhoneType type)
{
	return type == PhoneType::iOSRtmp || type == PhoneType::AndroidRtmp;
}

struct media_status_info {
	PhoneType type;
	media_status status;
};

enum class MsgType {
	iOSDeviceList,
	AndroidDeviceList,
	AndroidADBDeviceList,
	DeviceListResult,
	MediaTask,
};
