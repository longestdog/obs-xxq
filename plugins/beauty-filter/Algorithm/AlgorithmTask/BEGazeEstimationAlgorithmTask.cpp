#include "BEGazeEstimationAlgorithmTask.h"
#include "BETaskFactory.h"
#include "be_util.h"
#include "BEFaceAlgorithmTask.h"

BEGazeEstiMationAlgorithmTask::BEGazeEstiMationAlgorithmTask() {
	m_dependencys.push_back(BEFaceAlgorithmTask::face106Key());
}

BEGazeEstiMationAlgorithmTask::~BEGazeEstiMationAlgorithmTask() {

}

BETaskKey* BEGazeEstiMationAlgorithmTask::gazeKey() {
	TASK_KEY_CREATE_AND_REGISTER(GAZE_ESTIMATION, BEGazeEstiMationAlgorithmTask);
}

void BEGazeEstiMationAlgorithmTask::initTask() {
	bef_effect_result_t ret;

	ret = bef_effect_ai_gaze_estimation_create_handle(&m_handle);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_gaze_estimation_create_handle");
	ret = bef_effect_ai_gaze_estimation_check_license(m_handle, getLicensePath().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_gaze_estimation_check_license");
	ret = bef_effect_ai_gaze_estimation_init_model(m_handle, BEF_GAZE_ESTIMATION_MODEL1, getModel().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_gaze_estimation_init_model");
}

void BEGazeEstiMationAlgorithmTask::destroyTask() {
	bef_effect_ai_gaze_estimation_destroy(m_handle);
}

int BEGazeEstiMationAlgorithmTask::getPriority() {
	return 1000;
}

BETaskKey* BEGazeEstiMationAlgorithmTask::getKey() {
	return BEGazeEstiMationAlgorithmTask::gazeKey();
}

void BEGazeEstiMationAlgorithmTask::rebuild() {

}

void BEGazeEstiMationAlgorithmTask::process(BEProcessInput* input, BEProcessOutput* output) {
	memset(&m_gazeInfo, 0, sizeof(bef_ai_gaze_estimation_info));
	bef_ai_face_info* faceInfo = input->m_faceInfo;
	bef_effect_result_t ret;
	if (faceInfo != nullptr && faceInfo->face_count > 0) {
		START_ALG_TIMER("gazeEstimation");
		ret = bef_effect_ai_gaze_estimation_detect(m_handle,
			input->m_buffer,
			input->m_format,
			input->m_width,
			input->m_height,
			input->m_bytePerRow,
			input->m_ori,
			faceInfo,
			0,
			&m_gazeInfo);
		STOP_ALG_TIMER();
		CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_gaze_estimation_detect");
	}

	output->m_gazeInfo = &m_gazeInfo;
}

std::string BEGazeEstiMationAlgorithmTask::getModel() {
	return m_provider->getModelDir() + "gazeestimationmodel/algo_gglap_h_v3.0.model";
}