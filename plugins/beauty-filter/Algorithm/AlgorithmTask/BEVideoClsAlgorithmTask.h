#ifndef BEVIDEOCLS_ALGORITHM_TASK_H
#define BEVIDEOCLS_ALGORITHM_TASK_H

#include "BEAlgorithmTask.h"

class BEVideoClsAlgorithmTask : public BEAlgorithmTask {
public:
	BEVideoClsAlgorithmTask();
	~BEVideoClsAlgorithmTask();

	static BETaskKey* videoClsKey();
public:
	void initTask();
	void destroyTask();

	int getPriority();
	BETaskKey* getKey();

	void rebuild();
	void process(BEProcessInput* input, BEProcessOutput* output);

private:
	std::string getModel();

private:
	bef_ai_video_cls_handle m_handle = 0;
	bef_ai_video_cls_ret m_videoClsInfo;
	int m_frameCount = 0;
};


#endif
