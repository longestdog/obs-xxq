#include "BEFaceAlgorithmTask.h"
#include "BETaskFactory.h"
#include "be_util.h"

BEFaceAlgorithmTask::BEFaceAlgorithmTask() {
	m_detectConfig = BEF_DETECT_MODE_VIDEO | BEF_DETECT_FULL;
}

BEFaceAlgorithmTask::~BEFaceAlgorithmTask() {

}

BETaskKey* BEFaceAlgorithmTask::face106Key() {
	TASK_KEY_CREATE_AND_REGISTER(FACE_106, BEFaceAlgorithmTask);
}

BETaskKey* BEFaceAlgorithmTask::face280Key() {
	TASK_KEY_CREATE(FACE_280);
}

BETaskKey* BEFaceAlgorithmTask::faceMaskKey() {
	TASK_KEY_CREATE(FACE_MASK);
}

BETaskKey* BEFaceAlgorithmTask::mouthMaskKey() {
	TASK_KEY_CREATE(MOUTH_MASK);
}

BETaskKey* BEFaceAlgorithmTask::teethMaskKey() {
	TASK_KEY_CREATE(TEETH_MASK);
}

void BEFaceAlgorithmTask::initTask() {
	bef_effect_result_t ret = 0;
	//load face detect model
	unsigned long long config = BEF_DETECT_SMALL_MODEL | BEF_DETECT_FULL;
	if (getImageMode()) {
		config |= BEF_DETECT_MODE_IMAGE_SLOW;
	}
	else {
		config |= BEF_DETECT_MODE_VIDEO;
	}
	ret = bef_effect_ai_face_detect_create(config, getFaceModel().c_str(), &m_handle);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_face_detect_create");
	ret = bef_effect_ai_face_check_license(m_handle, getLicensePath().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_face_check_license");
	ret = bef_effect_ai_face_detect_setparam(m_handle, BEF_FACE_PARAM_FACE_DETECT_INTERVAL, 15);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_face_detect_setparam");
	ret = bef_effect_ai_face_detect_setparam(m_handle, BEF_FACE_PARAM_MAX_FACE_NUM, 15);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_face_detect_setparam");

	//load face detect model with 280 keypoints
	ret = bef_effect_ai_face_detect_add_extra_model(m_handle, TT_MOBILE_FACE_280_DETECT, getFaceExtraModel().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeFaceDetectHandle bef_effect_ai_face_detect_add_extra_model");
}

void BEFaceAlgorithmTask::destroyTask() {
	bef_effect_ai_face_detect_destroy(m_handle);
}

int BEFaceAlgorithmTask::getPriority() {
	return 500;
}

BETaskKey* BEFaceAlgorithmTask::getKey() {
	//TASK_KEY_CREATE_TRUE(FACE_106);
	return BEFaceAlgorithmTask::face106Key();
}

void BEFaceAlgorithmTask::process(BEProcessInput* input, BEProcessOutput* output) {
	if (m_bImageMode) {
		m_detectConfig |= BEF_DETECT_MODE_IMAGE_SLOW;
	}
	bef_effect_result_t ret;
	{
		START_ALG_TIMER("detectFace");
		ret = bef_effect_ai_face_detect(m_handle,
			input->m_buffer,
			input->m_format,
			input->m_width,
			input->m_height,
			input->m_bytePerRow,
			input->m_ori, m_detectConfig, &m_faceInfo);
		CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::detectFace bef_effect_ai_face_detect");
		STOP_ALG_TIMER();
	}

	input->m_faceInfo = &m_faceInfo;

	if (hasConfig(BEFaceAlgorithmTask::face106Key())) {
		output->m_faceInfo = &m_faceInfo;
	}

	if (hasConfig(BEFaceAlgorithmTask::face280Key())) {
		output->m_hasFaceExtra = true;
	}

	if (hasConfig(BEFaceAlgorithmTask::mouthMaskKey()) && ((m_detectConfig & AI_MOUTH_MASK_DETECT) == AI_MOUTH_MASK_DETECT)) {
		memset(&m_faceMouthMask, 0, sizeof(bef_ai_mouth_mask_info));
		START_ALG_TIMER("mouth_mask_detect");
		ret = bef_effect_ai_face_mask_detect(m_handle, m_detectConfig, BEF_FACE_DETECT_MOUTH_MASK, &m_faceMouthMask);
		output->m_faceMouthMask = &m_faceMouthMask;
		STOP_ALG_TIMER();
	}

	if (hasConfig(BEFaceAlgorithmTask::teethMaskKey()) && ((m_detectConfig & AI_TEETH_MASK_DETECT) == AI_TEETH_MASK_DETECT)) {
		memset(&m_faceTeethMask, 0, sizeof(bef_ai_teeth_mask_info));
		START_ALG_TIMER("teeth_mask_detect");
		ret = bef_effect_ai_face_mask_detect(m_handle, m_detectConfig, BEF_FACE_DETECT_TEETH_MASK, &m_faceTeethMask);
		output->m_faceTeethMask = &m_faceTeethMask;
		STOP_ALG_TIMER();
	}

	if (hasConfig(BEFaceAlgorithmTask::faceMaskKey()) && ((m_detectConfig & AI_FACE_MASK_DETECT) == AI_FACE_MASK_DETECT)) {
		memset(&m_faceMask, 0, sizeof(bef_ai_face_mask_info));
		START_ALG_TIMER("face_mask_detect");
		ret = bef_effect_ai_face_mask_detect(m_handle, m_detectConfig, BEF_FACE_DETECT_FACE_MASK, &m_faceMask);
		output->m_faceRestMask = &m_faceMask;
		STOP_ALG_TIMER();
	}

	return;
}

void BEFaceAlgorithmTask::rebuild() {
	calcConfig();
}

void BEFaceAlgorithmTask::setConfig(std::unordered_map<BETaskKey*, bool>& configMap) {
	__super::setConfig(configMap);

	calcConfig();
}

void BEFaceAlgorithmTask::calcConfig() {
	m_detectConfig = BEF_DETECT_MODE_VIDEO | BEF_DETECT_FULL;

	if (hasConfig(BEFaceAlgorithmTask::face280Key())) {
		m_detectConfig = BEF_DETECT_MODE_VIDEO | BEF_DETECT_FULL | TT_MOBILE_FACE_280_DETECT;
	}

	if (hasConfig(BEFaceAlgorithmTask::faceMaskKey())) {
		m_detectConfig |= TT_MOBILE_FACE_240_DETECT | AI_FACE_MASK_DETECT;
	}

	if (hasConfig(BEFaceAlgorithmTask::mouthMaskKey())) {
		m_detectConfig |= TT_MOBILE_FACE_240_DETECT | AI_MOUTH_MASK_DETECT;
	}

	if (hasConfig(BEFaceAlgorithmTask::teethMaskKey())) {
		m_detectConfig |= TT_MOBILE_FACE_240_DETECT | AI_TEETH_MASK_DETECT;
	}
}

std::string BEFaceAlgorithmTask::getFaceModel() {
	return m_provider->getModelDir() + "ttfacemodel/algo_ggl1pqh_v11.1.model";
}

std::string BEFaceAlgorithmTask::getFaceExtraModel() {
	return m_provider->getModelDir() + "ttfacemodel/algo_ggl1pqhlhmg7p_v14.0.model";
}
