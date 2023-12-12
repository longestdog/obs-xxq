#include "BESkeletonAlgorithmTask.h"
#include "BETaskFactory.h"
#include "be_util.h"


BESkeletonAlgorithmTask::BESkeletonAlgorithmTask() {

}

BESkeletonAlgorithmTask::~BESkeletonAlgorithmTask() {
	if (m_skeletonInfo) {
		delete[] m_skeletonInfo;
	}
}

BETaskKey* BESkeletonAlgorithmTask::skeletonKey() {
	TASK_KEY_CREATE_AND_REGISTER(SKELETON, BESkeletonAlgorithmTask);
}

void BESkeletonAlgorithmTask::initTask() {
	bef_effect_result_t ret;
	ret = bef_effect_ai_skeleton_create(getModel().c_str(), &m_handle);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_skeleton_create");

	ret = bef_effect_ai_skeleton_check_license(m_handle, getLicensePath().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_skeleton_check_license");
	ret = bef_effect_ai_skeleton_set_targetnum(m_handle, 1);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_skeleton_set_targetnum");
	m_skeletonInfo = new bef_ai_skeleton_info[BEF_AI_MAX_SKELETON_NUM];
}

void BESkeletonAlgorithmTask::destroyTask() {
	bef_effect_ai_skeleton_destroy(m_handle);
}

void BESkeletonAlgorithmTask::process(BEProcessInput* input, BEProcessOutput* output) {
	int validCount = 0;

	bef_effect_result_t ret;
	START_ALG_TIMER("detectSkeleton");
	ret = bef_effect_ai_skeleton_detect(m_handle,
		input->m_buffer,
		input->m_format,
		input->m_width,
		input->m_height,
		input->m_bytePerRow,
		input->m_ori,
		&validCount, &m_skeletonInfo);
	STOP_ALG_TIMER();


	output->m_skeletonInfo = m_skeletonInfo;
	output->skeletonCount = validCount;
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_skeleton_detect");
}

int BESkeletonAlgorithmTask::getPriority() {
	return 1000;
}

BETaskKey* BESkeletonAlgorithmTask::getKey() {
	return BESkeletonAlgorithmTask::skeletonKey();
}

void BESkeletonAlgorithmTask::rebuild() {

}

std::string BESkeletonAlgorithmTask::getModel() {
	return m_provider->getModelDir() + "skeleton_model/algo_gglush9hgtc_v7.0.model";
}
