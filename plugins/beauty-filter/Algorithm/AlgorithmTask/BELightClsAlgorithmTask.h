#ifndef BELIGHT_CLS_ALGORITHM_TASK_H
#define BELIGHT_CLS_ALGORITHM_TASK_H

#include "BEAlgorithmTask.h"

class BELightClsAlgorithmTask : public BEAlgorithmTask {
public:
	BELightClsAlgorithmTask();
	~BELightClsAlgorithmTask();

	static BETaskKey* lightClsKey();
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
	bef_effect_handle_t m_handle;
	bef_ai_light_cls_result m_lightClsInfo;
};

#endif
