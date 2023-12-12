#ifndef BEC2_ALGORITHM_TASK_H
#define BEC2_ALGORITHM_TASK_H

#include "BEAlgorithmTask.h"

class BEC2AlgorithmTask : public BEAlgorithmTask {
public:
	BEC2AlgorithmTask();
	~BEC2AlgorithmTask();

	static BETaskKey* c2Key();
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
	bef_ai_c2_handle m_handle;
	bef_ai_c2_ret* m_c2Info = nullptr;
};

#endif
