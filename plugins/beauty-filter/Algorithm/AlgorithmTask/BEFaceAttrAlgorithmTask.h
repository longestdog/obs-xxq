#ifndef BE_FACEATTR_ALGORITHM_TASK_H
#define BE_FACEATTR_ALGORITHM_TASK_H


#include "BEAlgorithmTask.h"

class BEFaceAttrAlgorithmTask : public BEAlgorithmTask {
public:
	BEFaceAttrAlgorithmTask();
	~BEFaceAttrAlgorithmTask();

	static BETaskKey* faceAttrKey();
public:
	void initTask();
	void destroyTask();

	int getPriority();
	BETaskKey* getKey();

	void rebuild();
	void process(BEProcessInput* input, BEProcessOutput* output);

	void calcConfig();
private:
	std::string getModel();

private:
	bef_effect_handle_t m_handle;

	bef_ai_face_attribute_result* m_faceAttrInfo{ nullptr };
};

#endif
