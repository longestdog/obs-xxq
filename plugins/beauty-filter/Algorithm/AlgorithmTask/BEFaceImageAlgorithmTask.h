#ifndef BEFACE_IMAGE_ALGORITHM_TASK_H
#define BEFACE_IMAGE_ALGORITHM_TASK_H


#include "BEAlgorithmTask.h"

class BEFaceImageAlgorithmTask : public BEAlgorithmTask {
public:
	BEFaceImageAlgorithmTask();
	~BEFaceImageAlgorithmTask();

	static BETaskKey* faceImageKey();
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
	unsigned long long m_config;
	bef_ai_face_info m_faceInfo;
};

#endif
