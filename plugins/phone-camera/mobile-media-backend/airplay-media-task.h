#pragma once

#include "base-media-task.h"

class AirplayMediaTask : public BaseMediaTask {
	Q_OBJECT
public:
	AirplayMediaTask(QObject *parent = nullptr);
	~AirplayMediaTask();

private:
	bool m_serverStarted = false;
};
