//
// Created by zhugejingjing on 2020/1/14.
//
#include "BEFPlatformDefine.h"

#ifndef BEF_COLORCONVERTER_H
#define BEF_COLORCONVERTER_H

#include <string>
#include <unordered_map>

#if USE_DYNAMIC_ANGLE
#include "gles_loader_autogen.h"
#elif BEF_EFFECT_PLATFORM == BEF_EFFECT_PLATFORM_WINDOWS || BEF_EFFECT_PLATFORM == BEF_EFFECT_PLATFORM_LINUX || BEF_EFFECT_PLATFORM == BEF_EFFECT_PLATFORM_ANDROID || BEF_EFFECT_PLATFORM == BEF_EFFECT_PLATFORM_WASM
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#elif BEF_EFFECT_PLATFORM == BEF_EFFECT_PLATFORM_MAC
#include <OpenGL/gl.h>
#elif BEF_EFFECT_PLATFORM == BEF_EFFECT_PLATFORM_IOS
#include <OpenGLES/ES2/gl.h>
#endif

NAMESPACE_BEF_EFFECT_FRAMEWORK_BEGIN

typedef enum {
	unknown = 0,
	color_rgba,
	color_rgb,
	color_bgra,
	color_bgr,
	color_yuy2,
	color_i420
} bef_color_type;
typedef enum {
	Default_Converter = 0,
	BGRA_RGBA,
	YUY2_RGBA,
	I420_RGBA,
	RGBA_I420,
	I420_RGBA_3C
} bef_converter_type;

enum VIDEO_RANGE
{
	FULL,
	PARTIAL
};
enum COLOR_SPACE
{
	JPEG,
	HD_REC709,
	SD_REC601
};

class BEFColorConverter
{
public:
	BEFColorConverter();
	virtual ~BEFColorConverter();
	int init();
	void destroy();
	unsigned int createTexture(int min_filter = GL_LINEAR, int mag_filter = GL_LINEAR, int wrap_s = GL_CLAMP_TO_EDGE, int wrap_t = GL_CLAMP_TO_EDGE);//返回TextureID
	int updateTexture(unsigned int textureID, void* buffer, int width, int height, bef_color_type format);
	int deleteTexture(unsigned int textureID);
	int readPixel(void* buffer, int width, int height, bef_color_type format);
	int readPixel(unsigned int textureID, void* buffer, int width, int height, bef_color_type format);
	virtual int convert(unsigned int srcTexture, unsigned int dstTexture, int width, int height);
	int setParam(const char* key, int value);

protected:

	virtual const char* getVertexShader();
	virtual const char* getFragmentShader();
	virtual int addExtraUniform() { return 0; };

	struct EffectGLEnv;
	EffectGLEnv* m_effectGLEnv = nullptr;
	std::unordered_map<std::string, int> m_param;
	bef_converter_type m_type;

};

class BEFColorConverterFactory {
public:
	static BEFColorConverter* create(bef_color_type inputType, bef_color_type outputType);
};

NAMESPACE_BEF_EFFECT_FRAMEWORK_END

#endif //BEF_COLORCONVERTER_H
