#include "BEHairParseAlgorithmTask.h"
#include "BETaskFactory.h"
#include "be_util.h"


BEHairParseAlgorithmTask::BEHairParseAlgorithmTask() {

}


BEHairParseAlgorithmTask::~BEHairParseAlgorithmTask() {

}

BETaskKey* BEHairParseAlgorithmTask::hairKey() {
	TASK_KEY_CREATE_AND_REGISTER(HAIR_PARSE, BEHairParseAlgorithmTask);
}


void BEHairParseAlgorithmTask::initTask() {
	bef_effect_result_t ret;
	//create hair parse handle
	ret = bef_effect_ai_hairparser_create(&m_handle);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeHairParseHandle bef_effect_ai_hairparser_create");

	//check license for hair parse
	ret = bef_effect_ai_hairparser_check_license(m_handle, getLicensePath().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeHairParseHandle bef_effect_ai_hairparser_check_license");

	//init the handle with hair parse model
	ret = bef_effect_ai_hairparser_init_model(m_handle, getModel().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeHairParseHandle bef_effect_ai_hairparser_init_model");

	//set param width = 128, height = 224, the others set true
	ret = bef_effect_ai_hairparser_set_param(m_handle, 128, 224, true, true);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeHairParseHandle bef_effect_ai_hairparser_set_param");
}

void BEHairParseAlgorithmTask::destroyTask() {
	if (m_hairParseInfo != nullptr) {
		free(m_hairParseInfo);
	}
	bef_effect_ai_hairparser_destroy(m_handle);
}

int BEHairParseAlgorithmTask::getPriority() {
	return 1000;
}

BETaskKey* BEHairParseAlgorithmTask::getKey() {
	return BEHairParseAlgorithmTask::hairKey();
}

void BEHairParseAlgorithmTask::rebuild() {

}

void BEHairParseAlgorithmTask::process(BEProcessInput* input, BEProcessOutput* output) {
	bef_effect_result_t ret;

	ret = bef_effect_ai_hairparser_get_output_shape(m_handle, m_size, m_size + 1, m_size + 2);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_hairparser_get_output_shape");

	int largeWidth = input->m_width;
	int largeHeight = input->m_height;

	int smallWidth = m_size[0];
	int smallHeight = m_size[1];

	int maskSize = 0;
	if (((float)largeHeight / smallHeight) != ((float)largeWidth / smallWidth)) {
		int net_input_w = 0;
		int net_input_h = 0;
		bef_effect_ai_hairparser_get_output_shape_with_input(m_handle, largeWidth, largeHeight, &net_input_w, &net_input_h);
		maskSize = net_input_w * net_input_h;
		m_size[0] = net_input_w;
		m_size[1] = net_input_h;
		m_size[2] = 1;
	}
	else {
		maskSize = smallHeight * smallWidth;
	}

	if (m_hairInfoLen != m_size[0] * m_size[1] * m_size[2]) {
		if (m_hairParseInfo != nullptr) {
			free(m_hairParseInfo);
		}

		m_hairInfoLen = maskSize;
		m_hairParseInfo = (unsigned char*)malloc(m_hairInfoLen);
	}

	memset(m_hairParseInfo, 0, maskSize);
	START_ALG_TIMER("parseHair");
	ret = bef_effect_ai_hairparser_do_detect(m_handle,
		input->m_buffer,
		input->m_format,
		input->m_width,
		input->m_height,
		input->m_bytePerRow,
		input->m_ori, m_hairParseInfo, false);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_hairparser_do_detect");
	STOP_ALG_TIMER();

	output->m_hairParseInfo = m_hairParseInfo;
	output->m_hairParseSize = m_size;
}

std::string BEHairParseAlgorithmTask::getModel() {
	return m_provider->getModelDir() + "hairparser/algo_ggl0p57_v11.0.model";
}
