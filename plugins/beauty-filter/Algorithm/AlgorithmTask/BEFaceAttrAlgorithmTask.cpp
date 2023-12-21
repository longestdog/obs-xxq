#include "BEFaceAttrAlgorithmTask.h"
#include "BETaskFactory.h"
#include "be_util.h"
#include "BEFaceAlgorithmTask.h"


BEFaceAttrAlgorithmTask::BEFaceAttrAlgorithmTask() {
	m_dependencys.push_back(BEFaceAlgorithmTask::face106Key());
	m_faceAttrInfo = new bef_ai_face_attribute_result;
}

BEFaceAttrAlgorithmTask::~BEFaceAttrAlgorithmTask() {
	if (m_faceAttrInfo) {
		delete m_faceAttrInfo;
	}
}

BETaskKey* BEFaceAttrAlgorithmTask::faceAttrKey() {
	TASK_KEY_CREATE_AND_REGISTER(FACE_ATTR, BEFaceAttrAlgorithmTask);
}

void BEFaceAttrAlgorithmTask::initTask() {
	bef_effect_result_t ret = 0;
	// load face attribute detect model
	ret = bef_effect_ai_face_attribute_create(0, getModel().c_str(), &m_handle);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_face_attribute_create");
	ret = bef_effect_ai_face_attribute_check_license(m_handle, getLicensePath().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_face_attribute_check_license");
}

void BEFaceAttrAlgorithmTask::destroyTask() {
	bef_effect_ai_face_attribute_destroy(m_handle);
}

int BEFaceAttrAlgorithmTask::getPriority() {
	return 600;
}

BETaskKey* BEFaceAttrAlgorithmTask::getKey() {
	return BEFaceAttrAlgorithmTask::faceAttrKey();
}

void BEFaceAttrAlgorithmTask::rebuild() {

}

void BEFaceAttrAlgorithmTask::process(BEProcessInput* input, BEProcessOutput* output) {
	bef_ai_face_info* faceInfo = input->m_faceInfo;
	memset(m_faceAttrInfo, 0, sizeof(bef_ai_face_attribute_result));
	if (faceInfo != nullptr) {
		START_ALG_TIMER("detectFaceAttr");
		bef_effect_result_t ret;
		unsigned long long attriConfig = BEF_FACE_ATTRIBUTE_AGE | BEF_FACE_ATTRIBUTE_HAPPINESS | BEF_FACE_ATTRIBUTE_EXPRESSION | BEF_FACE_ATTRIBUTE_GENDER
			| BEF_FACE_ATTRIBUTE_ATTRACTIVE | BEF_FACE_ATTRIBUTE_CONFUSE;
		if (faceInfo->face_count == 1) {
			ret = bef_effect_ai_face_attribute_detect(m_handle,
				input->m_buffer,
				input->m_format,
				input->m_width,
				input->m_height,
				input->m_bytePerRow,
				faceInfo->base_infos,
				attriConfig,
				&m_faceAttrInfo->attr_info[0]);
			CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::detectFaceAttribute bef_effect_ai_face_attribute_detect");
		}
		else
		{
			ret = bef_effect_ai_face_attribute_detect_batch(m_handle,
				input->m_buffer,
				input->m_format,
				input->m_width,
				input->m_height,
				input->m_bytePerRow,
				faceInfo->base_infos,
				faceInfo->face_count,
				attriConfig, m_faceAttrInfo);
			CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::detectFaceAttribute bef_effect_ai_face_attribute_detect_batch");
		}
		STOP_ALG_TIMER();
	}
	output->m_faceAttrInfo = m_faceAttrInfo;
}

void BEFaceAttrAlgorithmTask::calcConfig() {

}

std::string BEFaceAttrAlgorithmTask::getModel() {
	return m_provider->getModelDir() + "ttfaceattrmodel/algo_ggl1pqhlpgg754kghlgt4_v7.0.model";
}
