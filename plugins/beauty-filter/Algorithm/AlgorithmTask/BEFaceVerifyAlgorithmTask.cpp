#include "BEFaceVerifyAlgorithmTask.h"
#include "BETaskFactory.h"
#include "be_util.h"
#include "BEFaceImageAlgorithmTask.h"

BEFaceVerifyAlgorithmTask::BEFaceVerifyAlgorithmTask() {
	m_dependencys.push_back(BEFaceImageAlgorithmTask::faceImageKey());
}

BEFaceVerifyAlgorithmTask::~BEFaceVerifyAlgorithmTask() {

}

BETaskKey* BEFaceVerifyAlgorithmTask::faceVerifyKey() {
	TASK_KEY_CREATE_AND_REGISTER(FACE_VERIFY, BEFaceVerifyAlgorithmTask);
}

BETaskKey* BEFaceVerifyAlgorithmTask::faceVerifyFeatureKey() {
	TASK_KEY_CREATE(FACE_VERIFY_FEATURE);
}

void BEFaceVerifyAlgorithmTask::initTask() {
	bef_effect_result_t ret;
	ret = bef_effect_ai_face_verify_create(getModel().c_str(), 1, &m_handle);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeFaceVerifyHandle bef_effect_ai_face_verify_create");
	ret = bef_effect_ai_face_verify_check_license(m_handle, getLicensePath().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::initializeFaceVerifyHandle bef_effect_ai_face_verify_check_license");
}

void BEFaceVerifyAlgorithmTask::destroyTask() {
	bef_effect_ai_face_verify_destroy(m_handle);
}

void BEFaceVerifyAlgorithmTask::reset() {
	m_bValid = false;
	m_faceVerifyNum = 0;
}

int BEFaceVerifyAlgorithmTask::getPriority() {
	return 1000;
}

BETaskKey* BEFaceVerifyAlgorithmTask::getKey() {
	return BEFaceVerifyAlgorithmTask::faceVerifyKey();
}

void BEFaceVerifyAlgorithmTask::rebuild() {
	calcConfig();
}

void BEFaceVerifyAlgorithmTask::process(BEProcessInput* input, BEProcessOutput* output) {
	bef_ai_face_info* faceInfo = input->m_faceImageInfo;
	if (m_isVerify) {
		if (faceInfo != nullptr && faceInfo->face_count == 1) {
			m_bValid = true;
		}
		else {
			m_bValid = false;
		}
	}

	double similarity = 0.0;

	if (faceInfo != nullptr && faceInfo->face_count > 0) {
		if (m_isVerify) {
			if (m_bValid) {
				bef_effect_result_t ret = bef_effect_ai_face_extract_feature_single(
					m_handle,
					input->m_buffer,
					input->m_format,
					input->m_width,
					input->m_height,
					input->m_bytePerRow,
					input->m_ori,
					faceInfo->base_infos,
					m_currentVerifyFeature);
				CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_face_extract_feature_single");
			}
			m_isVerify = false;
		}
		else {
			if (m_bValid) {
				if (m_faceVerifyNum % 15 == 0) {
					START_ALG_TIMER("faceVerify");
					float destFaceVerifyFeature[BEF_AI_FACE_FEATURE_DIM];
					bef_effect_result_t ret = bef_effect_ai_face_extract_feature_single(
						m_handle,
						input->m_buffer,
						input->m_format,
						input->m_width,
						input->m_height,
						input->m_bytePerRow,
						input->m_ori,
						faceInfo->base_infos, destFaceVerifyFeature);
					CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_face_extract_feature_single");
					double featureDistance = bef_effect_ai_face_verify(m_currentVerifyFeature, destFaceVerifyFeature, BEF_AI_FACE_FEATURE_DIM);
					similarity = bef_effect_ai__dist2score(featureDistance);
					output->m_faceVerifyFaceInfo = faceInfo;
					output->m_faceVerifySimilarity = similarity;
					STOP_ALG_TIMER();
				}
				m_faceVerifyNum++;

				if (m_faceVerifyNum > 1500) {
					m_faceVerifyNum = 0;
				}
			}
		}
	}


}

void BEFaceVerifyAlgorithmTask::calcConfig() {
	m_isVerify = hasConfig(BEFaceVerifyAlgorithmTask::faceVerifyFeatureKey());
}

std::string BEFaceVerifyAlgorithmTask::getModel() {
	return m_provider->getModelDir() + "faceverifymodel/algo_ggl1pqhrh7516_v7.0.model";
}
