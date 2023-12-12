#ifndef BE_PROCESS_OUTPUT_H
#define BE_PROCESS_OUTPUT_H

#include "bef_effect_ai_public_define.h"
#include "bef_effect_ai_hand.h"
#include "bef_effect_ai_face_detect.h"
#include "bef_effect_ai_skeleton.h"
#include "bef_effect_ai_face_attribute.h"
#include "bef_effect_ai_hairparser.h"
#include "bef_effect_ai_portrait_matting.h"
#include "bef_effect_ai_face_verify.h"
#include "bef_effect_ai_headseg.h"
#include "bef_effect_ai_lightcls.h"
#include "bef_effect_ai_pet_face.h"
#include "bef_effect_ai_c1.h"
#include "bef_effect_ai_c2.h"
#include "bef_effect_ai_video_cls.h"
#include "bef_effect_ai_gaze_estimation.h"
#include "bef_effect_ai_headseg.h"
#include "bef_effect_ai_chroma_keying.h"
#include "AlgorithmDefine.h"


class ALGORITHM_CPP_API BEProcessInput {
public:
	BEProcessInput() {
		clear();
	}
	virtual ~BEProcessInput() {

	}

	void clear() {
		m_faceInfo = nullptr;
		m_faceImageInfo = nullptr;
		m_faceAttrInfo = nullptr;
	}
public:
	const unsigned char* m_buffer;
	int m_width;
	int m_height;
	int m_bytePerRow;
	bef_ai_rotate_type m_ori;
	bef_ai_pixel_format m_format;


	bef_ai_face_info* m_faceInfo;
	bef_ai_face_info* m_faceImageInfo;
	bef_ai_face_attribute_result* m_faceAttrInfo;
};

class BEProcessOutput {
public:
	BEProcessOutput() {
		clear();
	}
	virtual ~BEProcessOutput() {

	}

	void clear() {
		m_faceInfo = nullptr;
		m_faceImageInfo = nullptr;
		m_faceAttrInfo = nullptr;
		m_hasFaceExtra = false;

		m_faceMouthMask = nullptr;
		m_faceTeethMask = nullptr;
		m_faceRestMask = nullptr;
		m_handInfo = nullptr;

		m_skeletonInfo = nullptr;
		skeletonCount = 0;

		m_hairParseInfo = nullptr;
		m_hairParseSize = nullptr;

		m_portriatMatting = nullptr;
		m_portriatSize = nullptr;

		m_headSegInfo = nullptr;
		m_lightClsInfo = nullptr;
		m_c1Info = nullptr;
		m_c2Info = nullptr;
		m_chromaKeyingInfo = nullptr;
		m_videoClsInfo = nullptr;
		m_gazeInfo = nullptr;
		m_petFaceInfo = nullptr;
		m_faceVerifyFaceInfo = nullptr;
		m_faceVerifySimilarity = 0.0;
	}
public:
	bef_ai_face_info* m_faceInfo;
	bef_ai_face_info* m_faceImageInfo;
	bef_ai_face_attribute_result* m_faceAttrInfo;
	bool m_hasFaceExtra;

	bef_ai_mouth_mask_info* m_faceMouthMask;
	bef_ai_teeth_mask_info* m_faceTeethMask;
	bef_ai_face_mask_info* m_faceRestMask;
	bef_ai_hand_info* m_handInfo;

	bef_ai_skeleton_info* m_skeletonInfo;
	int skeletonCount;

	unsigned char* m_hairParseInfo;
	int* m_hairParseSize;

	unsigned char* m_portriatMatting;
	int* m_portriatSize;

	bef_ai_headseg_output* m_headSegInfo;
	bef_ai_light_cls_result* m_lightClsInfo;
	bef_ai_c1_output* m_c1Info;
	bef_ai_c2_ret* m_c2Info;
	bef_ai_chroma_keying_ret* m_chromaKeyingInfo;
	bef_ai_video_cls_ret* m_videoClsInfo;
	bef_ai_gaze_estimation_info* m_gazeInfo;
	bef_ai_pet_face_result* m_petFaceInfo;
	bef_ai_face_info* m_faceVerifyFaceInfo;
	double m_faceVerifySimilarity;		// 人脸比对相似度
};

#endif
