
#ifndef effectsSDK_effect_pixelformat_cvt_h
#define effectsSDK_effect_pixelformat_cvt_h

#include <stdio.h>
#ifdef __ANDROID__
#include <GLES2/gl2.h>
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2ext.h>

#define EGL_EGLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <EGL/eglext.h>
#if !BEF_EFFECT_ANDROID_WITH_JNI
#include <android/hardware_buffer.h>
#endif
#elif USE_ANGLE
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#elif BYTED_OPERATING_SYSTEM == BYTED_OPERATING_SYSTEM_MAC
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif
#include "bef_effect_ai_public_define.h"
#include "bef_effect_ai_log.h"

#define HAL_FIRST_FRAME_CNT 3

enum ConvertType{
  NV21ToRGBA,//android
  NV12ToRGBA,
  I420ToRGBA,//android camera2 format
  //nv12
  BT601ToRGBA,//ios
  BT601FullToRGBA,//ios
  BT709ToRGBA,//ios
  
  RGBAToNV12,
  RGBAToNV21,
  RGBAToI420,
};


#endif /* PixelFormatConvertor2_h */
