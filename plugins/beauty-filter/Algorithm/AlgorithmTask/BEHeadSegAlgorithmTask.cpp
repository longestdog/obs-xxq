#include "BEHeadSegAlgorithmTask.h"
#include "BETaskFactory.h"
#include "be_util.h"
#include "BEFaceAlgorithmTask.h"

static int HEADSEG_INPUT_WIDTH = 128;
static int HEADSEG_INPUT_HEIGHT = 128;

BEHeadSegAlgorithmTask::BEHeadSegAlgorithmTask() {
	m_dependencys.push_back(BEFaceAlgorithmTask::face106Key());
}

BEHeadSegAlgorithmTask::~BEHeadSegAlgorithmTask() {

}

BETaskKey* BEHeadSegAlgorithmTask::headKey() {
	TASK_KEY_CREATE_AND_REGISTER(HEAD_SEG, BEHeadSegAlgorithmTask);
}


void BEHeadSegAlgorithmTask::initTask() {
	bef_ai_headseg_config conf;
	conf.net_input_height = HEADSEG_INPUT_HEIGHT;
	conf.net_input_width = HEADSEG_INPUT_WIDTH;

	bef_effect_result_t ret = BEF_AI_HSeg_CreateHandler(&m_handle);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEF_AI_HSeg_CreateHandler");

	ret = BEF_AI_HSeg_CheckLicense(m_handle, getLicensePath().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEF_AI_HSeg_CheckLicense");

	ret = BEF_AI_HSeg_SetConfig(m_handle, &conf);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEF_AI_HSeg_SetConfig");

	ret = BEF_AI_HSeg_SetParam(m_handle, BEF_AI_HS_ENABLE_TRACKING, 1.0f);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEF_AI_HS_ENABLE_TRACKING");

	ret = BEF_AI_HSeg_SetParam(m_handle, BEF_AI_HS_MAX_FACE, 2);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEF_AI_HS_MAX_FACE");

	ret = BEF_AI_HSeg_InitModel(m_handle, getModel().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEF_AI_HSeg_InitModel");
}

void BEHeadSegAlgorithmTask::destroyTask() {
	BEF_AI_HSeg_ReleaseHandle(m_handle);
}

int BEHeadSegAlgorithmTask::getPriority() {
	return 1000;
}

BETaskKey* BEHeadSegAlgorithmTask::getKey() {
	return BEHeadSegAlgorithmTask::headKey();
}

void BEHeadSegAlgorithmTask::rebuild() {

}

void BEHeadSegAlgorithmTask::process(BEProcessInput* input, BEProcessOutput* output) {
	const bef_ai_face_info* faceInfo = input->m_faceInfo;

	if (faceInfo == nullptr) {
		return;
	}

	bef_effect_result_t ret;

	bef_ai_headseg_input inputArgs;
	inputArgs.face_count = faceInfo->face_count;
	inputArgs.image = const_cast<unsigned char*>(input->m_buffer);
	inputArgs.image_height = input->m_height;
	inputArgs.image_width = input->m_width;
	inputArgs.image_stride = input->m_bytePerRow;
	inputArgs.orient = (bef_ai_rotate_type)input->m_ori;
	inputArgs.pixel_format = (bef_ai_pixel_format)input->m_format;

	memset(&m_headSegInfo, 0, sizeof(bef_ai_headseg_output));
	if (inputArgs.face_count > 0) {
		bef_ai_headseg_faceinfo* headFaceInfo = new bef_ai_headseg_faceinfo[inputArgs.face_count];
		for (int i = 0; i < inputArgs.face_count; i++) {
			headFaceInfo[i].face_id = faceInfo->base_infos[i].ID;
			memcpy(headFaceInfo[i].points, faceInfo->base_infos[i].points_array, sizeof(float) * 2 * 106);
		}

		inputArgs.face_info = headFaceInfo;
		START_ALG_TIMER("headSegment");
		ret = BEF_AI_HSeg_DoHeadSeg(m_handle, &inputArgs, &m_headSegInfo);
		STOP_ALG_TIMER();
		delete[] headFaceInfo;

		CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEF_AI_HSeg_DoHeadSeg");
	}

	output->m_headSegInfo = &m_headSegInfo;
}

std::string BEHeadSegAlgorithmTask::getModel() {
	return m_provider->getModelDir() + "headsegmodel/algo_ggl0hpvuha_v6.0.model";
}