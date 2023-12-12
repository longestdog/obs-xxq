#include "BEPetFaceAlgorithmTask.h"
#include "BETaskFactory.h"
#include "be_util.h"

#define MAX_PET_FACE 10

BEPetFaceAlgorithmTask::BEPetFaceAlgorithmTask() {

}

BEPetFaceAlgorithmTask::~BEPetFaceAlgorithmTask() {

}

BETaskKey* BEPetFaceAlgorithmTask::petFaceKey() {
	TASK_KEY_CREATE_AND_REGISTER(PET_FACE, BEPetFaceAlgorithmTask);
}

void BEPetFaceAlgorithmTask::initTask() {
	bef_effect_result_t ret;
	ret = bef_effect_ai_pet_face_create(getModel().c_str(), BEF_DetCat | BEF_DetDog, MAX_PET_FACE, &m_handle);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_pet_face_create");
	ret = bef_effect_ai_pet_face_check_license(m_handle, getLicensePath().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_pet_face_check_license");
}

void BEPetFaceAlgorithmTask::destroyTask() {
	bef_effect_ai_pet_face_release(m_handle);
}

int BEPetFaceAlgorithmTask::getPriority() {
	return 1000;
}

BETaskKey* BEPetFaceAlgorithmTask::getKey() {
	return BEPetFaceAlgorithmTask::petFaceKey();
}

void BEPetFaceAlgorithmTask::rebuild() {

}

void BEPetFaceAlgorithmTask::process(BEProcessInput* input, BEProcessOutput* output) {
	bef_effect_result_t ret;
	START_ALG_TIMER("petFace");
	ret = bef_effect_ai_pet_face_detect(m_handle,
		input->m_buffer,
		input->m_format,
		input->m_width,
		input->m_height,
		input->m_bytePerRow,
		input->m_ori, &m_petFaceInfo);
	STOP_ALG_TIMER();
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "BEDectector::detectPetFace bef_effect_ai_pet_face_detect");

	output->m_petFaceInfo = &m_petFaceInfo;
}


std::string BEPetFaceAlgorithmTask::getModel() {
	return m_provider->getModelDir() + "petfacemodel/algo_gglihg1pqh_v5.2.model";
}
