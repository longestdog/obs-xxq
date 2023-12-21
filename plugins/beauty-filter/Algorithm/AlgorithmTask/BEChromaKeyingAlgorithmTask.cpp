#include "BEChromaKeyingAlgorithmTask.h"
#include "BETaskFactory.h"
#include "be_util.h"

BEChromaKeyingAlgorithmTask::BEChromaKeyingAlgorithmTask() {

}

BEChromaKeyingAlgorithmTask::~BEChromaKeyingAlgorithmTask() {

}

BETaskKey* BEChromaKeyingAlgorithmTask::chromaKeingKey() {
	TASK_KEY_CREATE_AND_REGISTER(chromaKeying, BEChromaKeyingAlgorithmTask);
}

BETaskKey* BEChromaKeyingAlgorithmTask::chromaGreenKeepModeKey() {
	TASK_KEY_CREATE(CHROMA_GREEN_KEEP_MODE);
}

void BEChromaKeyingAlgorithmTask::initTask() {
	bef_effect_result_t ret = bef_effect_ai_chroma_keying_create(&m_handle);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_chroma_keying_create");
	ret = bef_effect_ai_chroma_keying_check_license(m_handle, getLicensePath().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_chroma_keying_check_license");

}
void BEChromaKeyingAlgorithmTask::destroyTask() {
	bef_effect_ai_chroma_keying_release(m_handle);
}

int BEChromaKeyingAlgorithmTask::getPriority() {
	return 1000;
}

BETaskKey* BEChromaKeyingAlgorithmTask::getKey() {
	return BEChromaKeyingAlgorithmTask::chromaKeingKey();
}

void BEChromaKeyingAlgorithmTask::rebuild() {
	m_bIsGrrenKeepOn = hasConfig(BEChromaKeyingAlgorithmTask::chromaGreenKeepModeKey());
}

void BEChromaKeyingAlgorithmTask::process(BEProcessInput* input, BEProcessOutput* output) {
	if (!m_isInit) {
		// chromaketing process need glcontext
		int ret = bef_effect_ai_chroma_keying_init(m_handle, getModel().c_str());
		bef_effect_ai_chroma_keying_set_processParam(0.25, 0.5, 0.1, 0.5, 1.0);
		CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_chroma_keying_init");
		m_isInit = true;
	}


	START_ALG_TIMER("chromaKeying");

	unsigned char* data = nullptr;
	int width = input->m_width;
	int height = input->m_height;
	int stride = input->m_bytePerRow;
	bool imageWithPadding = (stride != width * 4);
	if (imageWithPadding) {
		data = (unsigned char*)malloc(width * height * 4);
		for (int i = 0; i < height; ++i) {
			memcpy(data + i * width * 4, input->m_buffer + i * stride, width * 4);
		}
		stride = width * 4;
	}


	bef_effect_result_t ret = bef_effect_ai_chroma_keying_detect(m_handle,
		imageWithPadding ? data : input->m_buffer,
		input->m_format,
		input->m_width,
		input->m_height,
		input->m_bytePerRow,
		input->m_ori, &m_chromaKeyingInfo,
		m_bIsGrrenKeepOn);
	STOP_ALG_TIMER();

	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_chroma_keying_detect");
	output->m_chromaKeyingInfo = &m_chromaKeyingInfo;
}

std::string BEChromaKeyingAlgorithmTask::getModel() {
	return m_provider->getModelDir() + "chroma_keying";
}