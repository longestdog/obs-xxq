#include "BEFaceImageAlgorithmTask.h"
#include "BETaskFactory.h"
#include "be_util.h"

BEFaceImageAlgorithmTask::BEFaceImageAlgorithmTask() {
	m_config = BEF_DETECT_SMALL_MODEL | BEF_DETECT_FULL | BEF_DETECT_MODE_IMAGE_SLOW;
}

BEFaceImageAlgorithmTask::~BEFaceImageAlgorithmTask() {

}

BETaskKey* BEFaceImageAlgorithmTask::faceImageKey() {
	TASK_KEY_CREATE_AND_REGISTER(FACE_IMAGE, BEFaceImageAlgorithmTask);
}

void BEFaceImageAlgorithmTask::initTask() {
	bef_effect_result_t ret = 0;
	//load face detect model
	ret = bef_effect_ai_face_detect_create(m_config, getModel().c_str(), &m_handle);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_face_detect_create");
	ret = bef_effect_ai_face_check_license(m_handle, getLicensePath().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_face_check_license");
	ret = bef_effect_ai_face_detect_setparam(m_handle, BEF_FACE_PARAM_MAX_FACE_NUM, 10);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_face_detect_setparam");
}

void BEFaceImageAlgorithmTask::destroyTask() {
	bef_effect_ai_face_detect_destroy(m_handle);
}

int BEFaceImageAlgorithmTask::getPriority() {
	return 999;
}

BETaskKey* BEFaceImageAlgorithmTask::getKey() {
	return BEFaceImageAlgorithmTask::faceImageKey();
}

void BEFaceImageAlgorithmTask::rebuild() {

}

void BEFaceImageAlgorithmTask::process(BEProcessInput* input, BEProcessOutput* output) {
	memset(&m_faceInfo, 0, sizeof(bef_ai_face_info));
	START_ALG_TIMER("detectFace");
	bef_effect_result_t ret = bef_effect_ai_face_detect(m_handle,
		input->m_buffer,
		input->m_format,
		input->m_width,
		input->m_height,
		input->m_bytePerRow,
		input->m_ori,
		m_config, &m_faceInfo);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "CHECK_BEF_AI_RET_SUCCESS_NO_RETURN");
	STOP_ALG_TIMER();
	input->m_faceImageInfo = &m_faceInfo;

	output->m_faceImageInfo = &m_faceInfo;
}

std::string BEFaceImageAlgorithmTask::getModel() {
	return m_provider->getModelDir() + "ttfacemodel/algo_ggl1pqh_v11.1.model";
}