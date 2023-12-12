#include "BELightClsAlgorithmTask.h"
#include "BETaskFactory.h"
#include "be_util.h"

BELightClsAlgorithmTask::BELightClsAlgorithmTask() {

}

BELightClsAlgorithmTask::~BELightClsAlgorithmTask() {

}

BETaskKey* BELightClsAlgorithmTask::lightClsKey() {
	TASK_KEY_CREATE_AND_REGISTER(LIGHT_CLS, BELightClsAlgorithmTask);
}

void BELightClsAlgorithmTask::initTask() {
	bef_effect_result_t ret;
	//create light classify handle
	int fps = 5;
	if (m_bImageMode) {
		fps = 1;
	}
	ret = bef_effect_ai_lightcls_create(&m_handle, getModel().c_str(), fps);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_light_classify_create_handle");

	//check license for light classify
	ret = bef_effect_ai_lightcls_check_license(m_handle, getLicensePath().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_light_classify_check_license");
}

void BELightClsAlgorithmTask::destroyTask() {
	bef_effect_ai_lightcls_release(m_handle);
}

int BELightClsAlgorithmTask::getPriority() {
	return 1000;
}

BETaskKey* BELightClsAlgorithmTask::getKey() {
	return BELightClsAlgorithmTask::lightClsKey();
}

void BELightClsAlgorithmTask::rebuild() {

}

void BELightClsAlgorithmTask::process(BEProcessInput* input, BEProcessOutput* output) {
	bef_effect_result_t ret;
	START_ALG_TIMER("detectLight");
	ret = bef_effect_ai_lightcls_detect(m_handle,
		input->m_buffer,
		input->m_format,
		input->m_width,
		input->m_height,
		input->m_bytePerRow,
		input->m_ori, &m_lightClsInfo);
	STOP_ALG_TIMER();
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_light_classify");
	output->m_lightClsInfo = &m_lightClsInfo;
}

std::string BELightClsAlgorithmTask::getModel() {
	return m_provider->getModelDir() + "lightcls/algo_ggl95a0gq9u_v1.0.model";
}