#include "BEVideoClsAlgorithmTask.h"
#include "BETaskFactory.h"
#include "be_util.h"

static const int FRAME_INTERVAL = 5;

BEVideoClsAlgorithmTask::BEVideoClsAlgorithmTask() {

}

BEVideoClsAlgorithmTask::~BEVideoClsAlgorithmTask() {

}

BETaskKey* BEVideoClsAlgorithmTask::videoClsKey() {
	TASK_KEY_CREATE_AND_REGISTER(VIDEO_CLS, BEVideoClsAlgorithmTask);
}

void BEVideoClsAlgorithmTask::initTask() {
	bef_effect_result_t ret = bef_effect_ai_video_cls_create(&m_handle);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_video_cls_create");
	ret = bef_effect_ai_video_cls_check_license(m_handle, getLicensePath().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_video_cls_check_license");
	ret = bef_effect_ai_video_cls_set_model(m_handle, BEF_AI_kVideoClsModel1, getModel().c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_video_cls_set_model");
}

void BEVideoClsAlgorithmTask::destroyTask() {
	bef_effect_ai_video_cls_release(m_handle);
}

int BEVideoClsAlgorithmTask::getPriority() {
	return 1000;
}

BETaskKey* BEVideoClsAlgorithmTask::getKey() {
	return BEVideoClsAlgorithmTask::videoClsKey();
}

void BEVideoClsAlgorithmTask::rebuild() {

}

void BEVideoClsAlgorithmTask::process(BEProcessInput* input, BEProcessOutput* output) {
	bef_ai_video_cls_args args;
	bef_ai_base_args base;
	base.image = input->m_buffer;
	base.image_width = input->m_width;
	base.image_height = input->m_height;
	base.pixel_fmt = input->m_format;
	base.orient = input->m_ori;
	base.image_stride = input->m_bytePerRow;
	args.bases = &base;
	args.is_last = (++m_frameCount % FRAME_INTERVAL) == 0;

	bef_effect_result_t ret;
	START_ALG_TIMER("videoCls");
	ret = bef_effect_ai_video_cls_detect(m_handle, &args, &m_videoClsInfo);
	STOP_ALG_TIMER();
	if (args.is_last) {
		output->m_videoClsInfo = &m_videoClsInfo;
	}
}

std::string BEVideoClsAlgorithmTask::getModel() {
	return m_provider->getModelDir() + "videoclsmodel/algo_gglr5vhtT9u_v4.0.model";
}
