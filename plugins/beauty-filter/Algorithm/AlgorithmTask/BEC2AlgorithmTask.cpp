#include "BEC2AlgorithmTask.h"
#include "BETaskFactory.h"
#include "be_util.h"

BEC2AlgorithmTask::BEC2AlgorithmTask() {

}

BEC2AlgorithmTask::~BEC2AlgorithmTask() {

}

BETaskKey* BEC2AlgorithmTask::c2Key() {
	TASK_KEY_CREATE_AND_REGISTER(C2, BEC2AlgorithmTask);
}


void BEC2AlgorithmTask::initTask() {
	bef_effect_result_t ret = bef_effect_ai_c2_create(&m_handle);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_c2_create");
	ret = bef_effect_ai_c2_check_license(m_handle, getLicensePath().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_c2_check_license");
	ret = bef_effect_ai_c2_set_model(m_handle, BEF_AI_kC2Model1, getModel().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_c2_set_model");
	m_c2Info = new bef_ai_c2_ret;
}

void BEC2AlgorithmTask::destroyTask() {
	if (m_c2Info) {
		delete m_c2Info;
		m_c2Info = nullptr;
	}
	bef_effect_ai_c2_release(m_handle);
}

int BEC2AlgorithmTask::getPriority() {
	return 1000;
}

BETaskKey* BEC2AlgorithmTask::getKey() {
	return BEC2AlgorithmTask::c2Key();
}

void BEC2AlgorithmTask::rebuild() {

}

void BEC2AlgorithmTask::process(BEProcessInput* input, BEProcessOutput* output) {
	START_ALG_TIMER("C2");
	bef_effect_result_t ret = bef_effect_ai_c2_detect(m_handle,
		input->m_buffer,
		input->m_format,
		input->m_width,
		input->m_height,
		input->m_bytePerRow,
		input->m_ori, m_c2Info);
	STOP_ALG_TIMER();

	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_c2_detect");
	output->m_c2Info = m_c2Info;
}

std::string BEC2AlgorithmTask::getModel() {
	return m_provider->getModelDir() + "c2";
}
