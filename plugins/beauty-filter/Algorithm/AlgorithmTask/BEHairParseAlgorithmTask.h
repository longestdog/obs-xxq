#ifndef BEHAIR_PARSE_ALGORITHM_TASK_H
#define BEHAIR_PARSE_ALGORITHM_TASK_H

#include "BEAlgorithmTask.h"


class BEHairParseAlgorithmTask : public BEAlgorithmTask {
public:
	BEHairParseAlgorithmTask();
	~BEHairParseAlgorithmTask();

	static BETaskKey* hairKey();
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
	int m_size[3];
	unsigned char* m_hairParseInfo = nullptr;
	int m_hairInfoLen = 0;
};

#endif