#ifndef BEPETFACE_ALGORITHM_TASK_H
#define BEPETFACE_ALGORITHM_TASK_H

#include "BEAlgorithmTask.h"

class BEPetFaceAlgorithmTask : public BEAlgorithmTask {
public:
	BEPetFaceAlgorithmTask();
	~BEPetFaceAlgorithmTask();

	static BETaskKey* petFaceKey();
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
	bef_effect_handle_t m_handle = 0;
	bef_ai_pet_face_result m_petFaceInfo;
};

#endif
