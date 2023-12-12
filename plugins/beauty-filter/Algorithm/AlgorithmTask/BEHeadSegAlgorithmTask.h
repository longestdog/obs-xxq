#ifndef BEHEADSEG_ALGORITHM_TASK_H
#define BEHEADSEG_ALGORITHM_TASK_H


#include "BEAlgorithmTask.h"

class BEHeadSegAlgorithmTask : public BEAlgorithmTask {
public:
	BEHeadSegAlgorithmTask();
	~BEHeadSegAlgorithmTask();

	static BETaskKey* headKey();
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
	bef_ai_headseg_handle m_handle;
	bef_ai_headseg_output m_headSegInfo;
};

#endif
