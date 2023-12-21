#ifndef BEC1_ALGORITHM_TASK_H
#define BEC1_ALGORITHM_TASK_H

#include "BEAlgorithmTask.h"

class BEC1AlgorithmTask : public BEAlgorithmTask {
public:
	BEC1AlgorithmTask();
	~BEC1AlgorithmTask();

	static BETaskKey* c1Key();
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
	bef_ai_c1_handle m_handle;
	bef_ai_c1_output m_c1Info;
};

#endif
