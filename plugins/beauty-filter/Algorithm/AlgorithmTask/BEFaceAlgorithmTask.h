#ifndef BEFACE_ALGORITHM_TASK_H
#define BEFACE_ALGORITHM_TASK_H

#include "BEAlgorithmTask.h"

class BEFaceAlgorithmTask : public BEAlgorithmTask {
public:
	BEFaceAlgorithmTask();
	~BEFaceAlgorithmTask();

	static BETaskKey* face106Key();
	static BETaskKey* face280Key();
	static BETaskKey* faceMaskKey();
	static BETaskKey* mouthMaskKey();
	static BETaskKey* teethMaskKey();

public:
	void initTask();
	void destroyTask();

	int getPriority();
	BETaskKey* getKey();

	void rebuild();
	void process(BEProcessInput* input, BEProcessOutput* output);
	void setConfig(std::unordered_map<BETaskKey*, bool>& configMap);

	void calcConfig();
private:
	std::string getFaceModel();
	std::string getFaceExtraModel();

private:
	bef_effect_handle_t m_handle;
	bef_ai_face_info m_faceInfo;

	bef_ai_mouth_mask_info m_faceMouthMask;
	bef_ai_teeth_mask_info m_faceTeethMask;
	bef_ai_face_mask_info m_faceMask;

	unsigned long long m_detectConfig;
};

#endif
