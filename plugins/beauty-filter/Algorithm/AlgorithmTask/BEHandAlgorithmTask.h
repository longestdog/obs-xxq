#ifndef BEHAND_ALGORITHM_TASK_H
#define BEHAND_ALGORITHM_TASK_H

#include "BEAlgorithmTask.h"

class BEHandAlgorithmTask : public BEAlgorithmTask {
public:
	BEHandAlgorithmTask();
	~BEHandAlgorithmTask();

	static BETaskKey* handKey();
public:
	virtual void initTask();
	void destroyTask();

	int getPriority();
	BETaskKey* getKey();

	void rebuild();
	void process(BEProcessInput* input, BEProcessOutput* output);

private:
	std::string getDetectModel();
	std::string getBoxModel();
	std::string getGestureModel();
	std::string getKeyPointModel();

private:
	bef_effect_handle_t m_handle;
	bef_ai_hand_info* m_handInfo{ nullptr };
	unsigned long long m_config;
};

#endif
