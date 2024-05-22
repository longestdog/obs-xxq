#pragma once

#include "base-media-task.h"

class RtmpMediaTask : public BaseMediaTask {
	Q_OBJECT
public:
	RtmpMediaTask(PhoneType type, QObject *parent = nullptr);
	~RtmpMediaTask();
	void init() override;
	void uninit() override;

private:
};
