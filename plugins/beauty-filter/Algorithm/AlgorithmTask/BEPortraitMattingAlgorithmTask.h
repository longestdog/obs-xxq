#ifndef BEPORTRAIT_MATTING_ALGORITHM_TASK_H
#define BEPORTRAIT_MATTING_ALGORITHM_TASK_H


#include "BEAlgorithmTask.h"

class BEPortraitMattingAlgorithm : public BEAlgorithmTask {
public:
	BEPortraitMattingAlgorithm();
	~BEPortraitMattingAlgorithm();

	static BETaskKey* portraitMattingKey();
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
	bef_effect_handle_t m_handle;
	int m_size[3];
	unsigned char* m_alpha = nullptr;
	int m_alphaLen = 0;
};

#endif
