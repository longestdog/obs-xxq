#ifndef EFFECT_MANAGER_DEFINE_H
#define EFFECT_MANAGER_DEFINE_H

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#if defined(EFFECT_MANAGER_EXPORT)
#define EFFECT_API extern "C" __declspec(dllexport)
#define EFFECT_CPP_API __declspec(dllexport)
#else
#define EFFECT_API extern "C" __declspec(dllimport)
#define EFFECT_CPP_API __declspec(dllimport)
#endif
#elif defined(__APPLE__)
#define EFFECT_API __attribute__((visibility("default"))) extern "C"
#define EFFECT_CPP_API __attribute__((visibility("default")))
#elif defined(__ANDROID__) || defined(__linux__)
#define EFFECT_API extern "C" __attribute__((visibility("default")))
#define EFFECT_CPP_API __attribute__((visibility("default")))
#else
#define EFFECT_API extern "C"
#define EFFECT_CPP_API
#endif

#endif