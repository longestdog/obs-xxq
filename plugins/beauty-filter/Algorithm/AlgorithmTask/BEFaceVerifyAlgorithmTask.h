#ifndef BEFACEVERIFY_ALGORITHM_TASK_H
#define BEFACEVERIFY_ALGORITHM_TASK_H


#include "BEAlgorithmTask.h"

class BEFaceVerifyAlgorithmTask : public BEAlgorithmTask {
public:
	BEFaceVerifyAlgorithmTask();
	~BEFaceVerifyAlgorithmTask();

	static BETaskKey* faceVerifyKey();
	static BETaskKey* faceVerifyFeatureKey();
public:
	void initTask();
	void destroyTask();

	void reset();

	int getPriority();
	BETaskKey* getKey();

	void rebuild();
	void process(BEProcessInput* input, BEProcessOutput* output);

	void calcConfig();
private:
	std::string getModel();

private:
	bef_effect_handle_t m_handle;
	bool m_isVerify = false;
	bool m_bValid = false;
	float m_currentVerifyFeature[BEF_AI_FACE_FEATURE_DIM];
	int m_faceVerifyNum = 0;
};


#endif
