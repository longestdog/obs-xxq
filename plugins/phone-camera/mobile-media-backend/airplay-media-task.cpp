#include "airplay-media-task.h"
#include <rpiplay.h>
#include <qdebug.h>

AirplayMediaTask::AirplayMediaTask(QObject *parent) : BaseMediaTask(PhoneType::iOSAirplay, parent) {}

AirplayMediaTask::~AirplayMediaTask() {}

void AirplayMediaTask::uninit()
{
	m_serverStarted =
		start_airplay_server([this](media_status status) { emit mediaState(status); },
				     [this](uint8_t *data, size_t size, int64_t ts, bool isVideo) { emit mediaData((char *)data, size, ts, isVideo); });
	if (!m_serverStarted)
		qDebug() << "AirplayMediaTask: fail to start airplay server";
}

void AirplayMediaTask::init()
{
	if (m_serverStarted)
		stop_airplay_server();
}
