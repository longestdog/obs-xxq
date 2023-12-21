#include "BEPortraitMattingAlgorithmTask.h"
#include "BETaskFactory.h"
#include "be_util.h"


BEPortraitMattingAlgorithm::BEPortraitMattingAlgorithm() {

}

BEPortraitMattingAlgorithm::~BEPortraitMattingAlgorithm() {

}

BETaskKey* BEPortraitMattingAlgorithm::portraitMattingKey() {
	TASK_KEY_CREATE_AND_REGISTER(PORTRIAT_MATTING, BEPortraitMattingAlgorithm);
}

void BEPortraitMattingAlgorithm::initTask() {
	bef_effect_result_t ret;
	// create portrait matting handle
	ret = bef_effect_ai_portrait_matting_create(&m_handle);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_portrait_matting_create");

	// check the  license of portrait matting
	ret = bef_effect_ai_matting_check_license(m_handle, getLicensePath().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_matting_check_license");

	// initialize the handle with portrait matting model
	ret = bef_effect_ai_portrait_matting_init_model(m_handle, BEF_MP_SUBJECT_MODEL, getModel().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_portrait_matting_init_model");

	//set param of model
	ret = bef_effect_ai_portrait_matting_set_param(m_handle, BEF_MP_EdgeMode, 1);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_portrait_matting_set_param");

	//set param of model
	ret = bef_effect_ai_portrait_matting_set_param(m_handle, BEF_MP_FrashEvery, 15);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_portrait_matting_set_param");

}

void BEPortraitMattingAlgorithm::destroyTask() {
	if (m_alpha != nullptr) {
		free(m_alpha);
		m_alpha = nullptr;
	}
	bef_effect_ai_portrait_matting_destroy(m_handle);
}

int BEPortraitMattingAlgorithm::getPriority() {
	return 1000;
}

BETaskKey* BEPortraitMattingAlgorithm::getKey() {
	return BEPortraitMattingAlgorithm::portraitMattingKey();
}

void BEPortraitMattingAlgorithm::rebuild() {

}

void BEPortraitMattingAlgorithm::process(BEProcessInput* input, BEProcessOutput* output) {
	bef_effect_result_t ret;
	int out_w = input->m_width;
	int out_h = input->m_height;
	bef_effect_ai_portrait_get_output_shape(m_handle, input->m_width, input->m_height, &out_w, &out_h);

	if (m_alphaLen != out_w * out_h) {
		if (m_alpha != nullptr) {
			free(m_alpha);
			m_alpha = nullptr;
		}
		m_alpha = (unsigned char*)malloc(out_w * out_h);
	}

	bef_ai_matting_ret mattingRet = { m_alpha, out_w, out_h };
	START_ALG_TIMER("detectMatting");
	ret = bef_effect_ai_portrait_matting_do_detect(m_handle,
		input->m_buffer,
		input->m_format,
		input->m_width,
		input->m_height,
		input->m_bytePerRow,
		input->m_ori,
		false, &mattingRet);
	STOP_ALG_TIMER();
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_portrait_matting_do_detect");

	m_size[0] = mattingRet.width;
	m_size[1] = mattingRet.height;
	m_size[2] = 1;

	output->m_portriatMatting = m_alpha;
	output->m_portriatSize = m_size;
}

std::string BEPortraitMattingAlgorithm::getModel() {
	//return m_provider->getModelDir() + "mattingmodel/tt_matting_v14.0.model";
	return m_provider->getModelDir() + "mattingmodel/algo_ggl8pgg5cal0pcvpka.model";
}
