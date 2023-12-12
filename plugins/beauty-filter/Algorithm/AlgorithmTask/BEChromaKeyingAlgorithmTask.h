#ifndef BE_CHROMAKEYING_ALGORITHM_TASK_H
#define BE_CHROMAKEYING_ALGORITHM_TASK_H

#include "BEAlgorithmTask.h"

class BEChromaKeyingAlgorithmTask : public BEAlgorithmTask {
public:
	BEChromaKeyingAlgorithmTask();
	~BEChromaKeyingAlgorithmTask();

	static BETaskKey* chromaKeingKey();
	static BETaskKey* chromaGreenKeepModeKey();
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
	bef_ai_chroma_keying_ret m_chromaKeyingInfo;
	bool m_isInit = false;
	bool m_bIsGrrenKeepOn = false;
};

#endif

