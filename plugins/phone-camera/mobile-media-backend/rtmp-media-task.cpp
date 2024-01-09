#include "rtmp-media-task.h"
#include <rtmp-main.h>
#include <qdebug.h>

RtmpMediaTask::RtmpMediaTask(PhoneType type, QObject *parent) : BaseMediaTask(type, parent)
{
	start_rtmp_server([this](media_status status) { emit mediaState(status); },
			  [this](uint8_t *data, size_t size, int64_t ts, bool isVideo) { emit mediaData((char *)data, size, ts, isVideo); });
}

RtmpMediaTask::~RtmpMediaTask()
{
	stop_rtmp_server();
}
