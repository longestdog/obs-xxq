#ifndef BESKELETON_ALGORITHM_TASK_H
#define BESKELETON_ALGORITHM_TASK_H

#include "BEAlgorithmTask.h"

class BESkeletonAlgorithmTask : public BEAlgorithmTask {
public:
	BESkeletonAlgorithmTask();
	~BESkeletonAlgorithmTask();

	static BETaskKey* skeletonKey();
public:
	virtual void initTask();
	void destroyTask();

	int getPriority();
	BETaskKey* getKey();

	void rebuild();
	void process(BEProcessInput* input, BEProcessOutput* output);

private:
	std::string getModel();

private:
	bef_effect_handle_t m_handle = 0;

	bef_ai_skeleton_info* m_skeletonInfo = nullptr;
};

#endif
