#include "BEC1AlgorithmTask.h"
#include "BETaskFactory.h"
#include "be_util.h"


BEC1AlgorithmTask::BEC1AlgorithmTask() {

}

BEC1AlgorithmTask::~BEC1AlgorithmTask() {

}

BETaskKey* BEC1AlgorithmTask::c1Key() {
	TASK_KEY_CREATE_AND_REGISTER(C1, BEC1AlgorithmTask);
}

void BEC1AlgorithmTask::initTask() {
	bef_effect_result_t ret = bef_effect_ai_c1_create(&m_handle, BEF_AI_C1_MODEL_SMALL, getModel().c_str());

	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_c1_create");
	ret = bef_effect_ai_c1_check_license(m_handle, getLicensePath().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_c1_check_license");
	ret = bef_effect_ai_c1_set_param(m_handle, BEF_AI_C1_USE_MultiLabels, 1);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_c1_set_param");
}

void BEC1AlgorithmTask::destroyTask() {
	bef_effect_ai_c1_release(m_handle);
}

int BEC1AlgorithmTask::getPriority() {
	return 1000;
}

BETaskKey* BEC1AlgorithmTask::getKey() {
	return BEC1AlgorithmTask::c1Key();
}

void BEC1AlgorithmTask::rebuild() {

}

void BEC1AlgorithmTask::process(BEProcessInput* input, BEProcessOutput* output) {
	START_ALG_TIMER("C1");
	bef_effect_result_t ret = bef_effect_ai_c1_detect(m_handle,
		input->m_buffer,
		input->m_format,
		input->m_width,
		input->m_height,
		input->m_bytePerRow,
		input->m_ori, &m_c1Info);
	STOP_ALG_TIMER();
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_c1_detect");


	output->m_c1Info = &m_c1Info;
}

std::string BEC1AlgorithmTask::getModel() {
	return m_provider->getModelDir() + "c1/algo_gglqjlu8p99_v8.0.model";
}
