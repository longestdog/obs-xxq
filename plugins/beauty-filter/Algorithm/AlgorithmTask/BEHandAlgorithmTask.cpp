#include "BEHandAlgorithmTask.h"
#include "BETaskFactory.h"
#include "be_util.h"

BEHandAlgorithmTask::BEHandAlgorithmTask() {
	m_config = BEF_AI_HAND_MODEL_DETECT | BEF_AI_HAND_MODEL_BOX_REG |
		BEF_AI_HAND_MODEL_GESTURE_CLS | BEF_AI_HAND_MODEL_KEY_POINT;
	m_handInfo = new bef_ai_hand_info;
}

BEHandAlgorithmTask::~BEHandAlgorithmTask() {
	if (m_handInfo) {
		delete m_handInfo;
	}
}

BETaskKey* BEHandAlgorithmTask::handKey() {
	TASK_KEY_CREATE_AND_REGISTER(HAND, BEHandAlgorithmTask);
}

void BEHandAlgorithmTask::initTask() {
	bef_effect_result_t ret;

	// create hand detect handle
	ret = bef_effect_ai_hand_detect_create(&m_handle, 0);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeHandDetectHandle bef_effect_ai_hand_detect_create");

	// check license for hand model
	ret = bef_effect_ai_hand_check_license(m_handle, getLicensePath().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_hand_check_license");

	//init hand detecting  with model
	ret = bef_effect_ai_hand_detect_setmodel(m_handle, BEF_AI_HAND_MODEL_DETECT, getDetectModel().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_hand_detect_setmodel");

	//init hand box detecting  with model
	ret = bef_effect_ai_hand_detect_setmodel(m_handle, BEF_AI_HAND_MODEL_BOX_REG, getBoxModel().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_hand_detect_setmodel");

	//init hand gesture detecting with model
	ret = bef_effect_ai_hand_detect_setmodel(m_handle, BEF_AI_HAND_MODEL_GESTURE_CLS, getGestureModel().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_hand_detect_setmodel");

	// init hand key point detecting with model
	ret = bef_effect_ai_hand_detect_setmodel(m_handle, BEF_AI_HAND_MODEL_KEY_POINT, getKeyPointModel().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_hand_detect_setmodel");

	// set max hand number for detect
	ret = bef_effect_ai_hand_detect_setparam(m_handle, BEF_HAND_MAX_HAND_NUM, 1);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_hand_detect_setparam");


	ret = bef_effect_ai_hand_detect_setparam(m_handle, BEF_HAND_NARUTO_GESTURE, 1);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_hand_detect_setparam");

	//set the enlarge ratio of initial rect for regression model
	ret = bef_effect_ai_hand_detect_setparam(m_handle, BEF_HNAD_ENLARGE_FACTOR_REG, 2.0);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_hand_detect_setparam");
}

void BEHandAlgorithmTask::destroyTask() {
	bef_effect_ai_hand_detect_destroy(m_handle);
}

int BEHandAlgorithmTask::getPriority() {
	return 1000;
}

BETaskKey* BEHandAlgorithmTask::getKey() {
	return BEHandAlgorithmTask::handKey();
}

void BEHandAlgorithmTask::rebuild() {

}

void BEHandAlgorithmTask::process(BEProcessInput* input, BEProcessOutput* output) {
	START_ALG_TIMER("detectHand");
	bef_effect_result_t ret = bef_effect_ai_hand_detect(m_handle,
		input->m_buffer,
		input->m_format,
		input->m_width,
		input->m_height,
		input->m_bytePerRow,
		input->m_ori, m_config, m_handInfo, 0);
	STOP_ALG_TIMER();
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_hand_detect");

	output->m_handInfo = m_handInfo;
}

std::string BEHandAlgorithmTask::getDetectModel() {
	return m_provider->getModelDir() + "handmodel/algo_ggl0pcvlvhg_v11.0.model";
}

std::string BEHandAlgorithmTask::getBoxModel() {
	return m_provider->getModelDir() + "handmodel/algo_ggl0pcvl4tml7ha_v12.0.model";
}

std::string BEHandAlgorithmTask::getGestureModel() {
	return m_provider->getModelDir() + "handmodel/algo_ggl0pcvlahugk7hlgt4_v11.2.model";
}

std::string BEHandAlgorithmTask::getKeyPointModel() {
	return m_provider->getModelDir() + "handmodel/algo_ggl0pcvlsi_v6.0.model";
}
