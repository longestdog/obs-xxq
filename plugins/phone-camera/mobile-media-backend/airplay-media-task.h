#pragma once

#include "base-media-task.h"

class AirplayMediaTask : public BaseMediaTask {
	Q_OBJECT
public:
	AirplayMediaTask(QObject *parent = nullptr);
	~AirplayMediaTask();
	void init() override;
	void uninit() override;

private:
	bool m_serverStarted = false;
};
