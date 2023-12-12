#ifndef ALGORITHM_DEFINE_H
#define ALGORITHM_DEFINE_H


#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#if defined(ALGORITHM_EXPORT)
#define ALGORITHM_API extern "C" __declspec(dllexport)
#define ALGORITHM_CPP_API __declspec(dllexport)
#else
#define ALGORITHM_API extern "C" __declspec(dllimport)
#define ALGORITHM_CPP_API __declspec(dllimport)
#endif
#elif defined(__APPLE__)
#define ALGORITHM_API __attribute__((visibility("default"))) extern "C"
#define ALGORITHM_CPP_API __attribute__((visibility("default")))
#elif defined(__ANDROID__) || defined(__linux__)
#define ALGORITHM_API extern "C" __attribute__((visibility("default")))
#define ALGORITHM_CPP_API __attribute__((visibility("default")))
#else
#define EFFECT_API extern "C"
#define EFFECT_CPP_API
#endif

enum BE_FEATURE : int {
	BE_FEATURE_UNKONWN = 0,
	BE_FEATURE_EFFECT = 1,
	BE_FEATURE_FACE = 2,
	BE_FEATURE_FACE_EXTRA = 3,
	BE_FEATURE_FACE_ATTRIBUTE = 4,
	BE_FEATURE_SKELETON = 5,
	BE_FEATURE_HAND_DETECT = 6,
	BE_FEATURE_HAND_GESTURE = 7,
	BE_FEATURE_HAND_SEG = 8,
	BE_FEATURE_HAND_KEY_POINT = 9,
	BE_FEATURE_HAND_BOX = 10,
	BE_FEATURE_HAIRPARSE = 11,
	BE_FEATURE_PORTRAIT_MATTING = 12,
	BE_FEATURE_FACE_VERIFY = 13,
	BE_FEATURE_LIGHT_CLASSIFY = 14,
	BE_FEATURE_FACE_MOUSE_MASK = 15,
	BE_FEATURE_FACE_TEETH_MASK = 16,
	BE_FEATURE_FACE_MASK = 17,
	BE_FEATURE_HEAD_SEG = 18,
	BE_FEATURE_C1 = 19,
	BE_FEATURE_C2 = 20,
	BE_FEATURE_VIDEO = 21,
	BE_FEATURE_GAZE_ESTIMATION = 22,
	BE_FEATURE_PET_FACE = 23,
	BE_FEATURE_CHROMAKEYING = 25,
	BE_FEATURE_CHROMAKEYING_SMOOTH = 26,
};

#endif
