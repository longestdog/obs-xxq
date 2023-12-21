#ifndef BEGAZE_ESTIMATION_ALGORITHM_TASK_H
#define BEGAZE_ESTIMATION_ALGORITHM_TASK_H

#include "BEAlgorithmTask.h"

class BEGazeEstiMationAlgorithmTask : public BEAlgorithmTask {
public:
	BEGazeEstiMationAlgorithmTask();
	~BEGazeEstiMationAlgorithmTask();

	static BETaskKey* gazeKey();
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
	bef_ai_gaze_handle m_handle;
	bef_ai_gaze_estimation_info m_gazeInfo;
};

#endif
