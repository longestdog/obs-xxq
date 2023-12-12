//
// Created by zhugejingjing on 2020/1/14.
//

#include "BEFColorConverter.h"
#if BEF_EFFECT_PLATFORM == BEF_EFFECT_PLATFORM_MAC
#include <OpenGL/gl3.h>
#endif

NAMESPACE_BEF_EFFECT_FRAMEWORK_BEGIN

struct BEFColorConverter::EffectGLEnv {
	GLenum m_inFormat = GL_RGBA; //input channel
	GLenum m_outFormat = GL_RGBA; //output channel
	int m_outputWidth = 0;
	int m_outputHeight = 0;
	GLuint m_fbo = 0;
	GLuint m_program = 0;
};

class BGRA_RGBAConverter : public BEFColorConverter
{
public:
	BGRA_RGBAConverter() { m_type = Default_Converter; };
protected:
	const char* getFragmentShader();
};

class YUV_RGBAConverter : public BEFColorConverter
{
public:
	YUV_RGBAConverter();
protected:
	virtual int addExtraUniform();
};

class YUY2_RGBAConverter : public YUV_RGBAConverter
{
public:
	YUY2_RGBAConverter() { m_type = YUY2_RGBA; };
protected:
	const char* getFragmentShader();
};

class I420_RGBAConverter : public YUV_RGBAConverter
{
public:
	I420_RGBAConverter() { m_type = I420_RGBA; };
protected:
	const char* getFragmentShader();
};

class RGBA_I420Converter : public YUV_RGBAConverter
{
public:
	RGBA_I420Converter() { m_type = RGBA_I420; };
protected:
	const char* getFragmentShader();
	virtual int addExtraUniform();
};

class RGBA_BGRAConverter : public BEFColorConverter
{
public:
	RGBA_BGRAConverter() { m_type = Default_Converter; };
protected:
	const char* getFragmentShader();
};

class I420_RGBA_3C_Converter : public YUV_RGBAConverter
{
public:
	I420_RGBA_3C_Converter() { m_type = I420_RGBA_3C; };
	virtual int convert(unsigned int srcY, unsigned int srcU, unsigned int srcB, unsigned int dstTexture, int width, int height);
protected:
	const char* getFragmentShader();
};


static const GLfloat VertexArray[] = {
	-1.0, -1.0, 0.0,
	1.0, -1.0, 0.0,
	-1.0, 1.0, 0.0,
	1.0, 1.0, 0.0,
};

static const GLfloat UVArray[] = {
	0.0f, 0.0f,
	1.0f, 0.0f,
	0.0f, 1.0f,
	1.0f, 1.0f,
};

const char* BEFColorConverter::getVertexShader() {
	return R"(
attribute vec3 attPosition;
attribute vec2 attUV;
varying vec2 textureCoordinate;
uniform float u_flipX;
uniform float u_flipY;
void main()
{
    gl_Position = vec4(attPosition.xy * vec2(u_flipX, u_flipY), attPosition.z, 1.);
    textureCoordinate = attUV;
}
)";
}

const char* BEFColorConverter::getFragmentShader() {
	return R"(
varying vec2 textureCoordinate;
uniform sampler2D u_inputTexture;

void main()
{
    vec4 color = texture2D(u_inputTexture, textureCoordinate);
    gl_FragColor = vec4(color.r, color.g, color.b, color.a);
}
)";
}

BEFColorConverter::BEFColorConverter() {
	m_param.insert(std::make_pair("flipX", 0));
	m_param.insert(std::make_pair("flipY", 0));
	m_effectGLEnv = new EffectGLEnv();
};

BEFColorConverter::~BEFColorConverter() {
	destroy();
};

int BEFColorConverter::init() {

	int ret = -1;
	GLuint vertexShader = 0;
	GLuint fragmentShader = 0;
	do {

		//FBO
		GLuint prevFbo = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&prevFbo);
		glGenFramebuffers(1, &m_effectGLEnv->m_fbo);

		//CompileShader
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		const char* vs = getVertexShader();
#if BEF_EFFECT_PLATFORM == BEF_EFFECT_PLATFORM_MAC
		std::string vsTmpStr = "#version 100\n" + std::string(vs);
		vs = vsTmpStr.c_str();
#endif
		glShaderSource(vertexShader, 1, &vs, nullptr);
		glCompileShader(vertexShader);
		GLint   compileStatus;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileStatus);
		if (!compileStatus)
			break;

		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		const char* fs = getFragmentShader();
#if BEF_EFFECT_PLATFORM == BEF_EFFECT_PLATFORM_MAC
		std::string tmpStr = "#version 100\nprecision mediump float;\n" + std::string(fs);
		fs = tmpStr.c_str();
#else 
		std::string tmpStr = "precision highp float;\n" + std::string(fs);
		fs = tmpStr.c_str();
#endif
		glShaderSource(fragmentShader, 1, &fs, nullptr);
		glCompileShader(fragmentShader);
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileStatus);
		if (!compileStatus)
			break;

		//CreateProgram
		m_effectGLEnv->m_program = glCreateProgram();
		glAttachShader(m_effectGLEnv->m_program, vertexShader);
		glAttachShader(m_effectGLEnv->m_program, fragmentShader);
		glLinkProgram(m_effectGLEnv->m_program);
		GLint linked;
		glGetProgramiv(m_effectGLEnv->m_program, GL_LINK_STATUS, &linked);
		if (!linked)
			break;

		ret = 0;
	} while (false);
	//release
	if (vertexShader)
		glDeleteShader(vertexShader);
	if (fragmentShader)
		glDeleteShader(fragmentShader);

	return ret;
}

void BEFColorConverter::destroy() {
	//destroy fbo
	if (m_effectGLEnv) {
		if (m_effectGLEnv->m_fbo) {
			glDeleteFramebuffers(1, &m_effectGLEnv->m_fbo);
		}
		//destroy program
		if (m_effectGLEnv->m_program)
			glDeleteProgram(m_effectGLEnv->m_program);
		delete m_effectGLEnv;
		m_effectGLEnv = nullptr;
	}
}

unsigned int BEFColorConverter::createTexture(int min_filter, int mag_filter, int wrap_s, int wrap_t) {
	unsigned int retTextureID = 0;
	glGenTextures(1, &retTextureID);
	glBindTexture(GL_TEXTURE_2D, retTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
	glBindTexture(GL_TEXTURE_2D, 0);
	return retTextureID;
}

int BEFColorConverter::updateTexture(unsigned int textureID, void* buffer, int width, int height, bef_color_type format) {
	GLenum m_format = GL_RGBA;
	int inputWidth = width;
	int inputHeight = height;
	switch (format) {
	case color_rgb:
	case color_bgr:
		m_format = GL_RGB;
		break;
	case color_i420:
#if BEF_EFFECT_PLATFORM == BEF_EFFECT_PLATFORM_IOS || BEF_EFFECT_PLATFORM == BEF_EFFECT_PLATFORM_WASM
		m_format = GL_ALPHA;
#else
		m_format = GL_RED;
#endif
		inputWidth = width;
		inputHeight = height * 3 / 2;
		break;
	case color_yuy2:
		inputWidth = width / 2;
		m_format = GL_RGBA;
		break;
	default:
		break;
	}
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, m_format, inputWidth, inputHeight, 0, m_format, GL_UNSIGNED_BYTE, buffer);
	GLenum ret = glGetError();
	glBindTexture(GL_TEXTURE_2D, 0);
	return 0;
}

int BEFColorConverter::deleteTexture(unsigned int textureID) {
	if (glIsTexture(textureID)) {
		glDeleteTextures(1, &textureID);
		return 0;
	}
	return -1;
}

int BEFColorConverter::readPixel(void* buffer, int width, int height, bef_color_type format) {
	int ret = -1;
	GLenum m_format = GL_RGBA;
	switch (format) {
	case color_rgb:
	case color_bgr:
		m_format = GL_RGB;
		break;
	case color_i420:
#if BEF_EFFECT_PLATFORM == BEF_EFFECT_PLATFORM_IOS || BEF_EFFECT_PLATFORM == BEF_EFFECT_PLATFORM_WASM
		m_format = GL_ALPHA;
#else
		m_format = GL_RED;
#endif
		height = height * 3 / 2;
		break;
	default:
		m_format = GL_RGBA;
	}
	//保存原fbo
	GLuint prevFbo = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&prevFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_effectGLEnv->m_fbo);
	glActiveTexture(GL_TEXTURE0);
	glViewport(0, 0, width, height);
	glReadPixels(0, 0, width, height, m_format, GL_UNSIGNED_BYTE, buffer);
	//恢复原fbo
	glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);

	ret = glGetError();
	return ret;
}

int BEFColorConverter::readPixel(unsigned int textureID, void* buffer, int width, int height, bef_color_type format) {
	int ret = -1;
	GLenum m_format = GL_RGBA;
	switch (format) {
	case color_rgb:
	case color_bgr:
		//m_format = GL_RGB;  // glReadPixels GL_RGB  return 1282;
		m_format = GL_RGBA;
		break;
	case color_i420:
#if BEF_EFFECT_PLATFORM == BEF_EFFECT_PLATFORM_IOS || BEF_EFFECT_PLATFORM == BEF_EFFECT_PLATFORM_WASM
		m_format = GL_ALPHA;
#else
		m_format = GL_RED;
#endif
		height = height * 3 / 2;
		break;
	default:
		m_format = GL_RGBA;
	}
	//保存原fbo
	GLuint prevFbo = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&prevFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_effectGLEnv->m_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
	glViewport(0, 0, width, height);
	glReadPixels(0, 0, width, height, m_format, GL_UNSIGNED_BYTE, buffer);
	//恢复原fbo
	glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);

	ret = glGetError();
	return ret;
}

int BEFColorConverter::convert(unsigned int srcTexture, unsigned int dstTexture, int width, int height) {
	int ret = -1;
	if (width <= 0 || height <= 0) {
		return -1;
	}
	m_effectGLEnv->m_outputWidth = width;
	m_effectGLEnv->m_outputHeight = height;
	//保存原fbo
	GLuint prevFbo = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&prevFbo);
	do {
		if (dstTexture != 0) {
			glBindFramebuffer(GL_FRAMEBUFFER, m_effectGLEnv->m_fbo);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dstTexture, 0);
		}
		else {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		glClearColor(1.0, 0.0, 0.0, 1.0);

		GLint _attPosition = glGetAttribLocation(m_effectGLEnv->m_program, "attPosition");
		if (-1 == _attPosition)
			break;
		GLint _attUV = glGetAttribLocation(m_effectGLEnv->m_program, "attUV");
		if (-1 == _attUV)
			break;
		GLint _inputTexture = glGetUniformLocation(m_effectGLEnv->m_program, "u_inputTexture");
		if (-1 == _inputTexture)
			break;
		GLint _flipX = glGetUniformLocation(m_effectGLEnv->m_program, "u_flipX");
		if (-1 == _flipX)
			break;
		GLint _flipY = glGetUniformLocation(m_effectGLEnv->m_program, "u_flipY");
		if (-1 == _flipY)
			break;

		//prepareProgram
		glUseProgram(m_effectGLEnv->m_program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, srcTexture);
		glUniform1i(_inputTexture, 0);
		if (m_param["flipX"]) {
			glUniform1f(_flipX, -1);
		}
		else {
			glUniform1f(_flipX, 1);
		}
		if (m_param["flipY"]) {
			glUniform1f(_flipY, -1);
		}
		else {
			glUniform1f(_flipY, 1);
		}
		GLuint _vao;
		glGenVertexArrays(1, &_vao);
		glBindVertexArray(_vao);
		GLuint _vboIds[2];;
		glGenBuffers(2, _vboIds);
		glBindBuffer(GL_ARRAY_BUFFER, _vboIds[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(VertexArray), VertexArray, GL_STATIC_DRAW);
		glEnableVertexAttribArray(_attPosition);
		//glVertexAttribPointer(_attPosition, 3, GL_FLOAT, 0, 0, VertexArray);
		glVertexAttribPointer(_attPosition, 3, GL_FLOAT, 0, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, _vboIds[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(UVArray), UVArray, GL_STATIC_DRAW);
		glEnableVertexAttribArray(_attUV);
		glVertexAttribPointer(_attUV, 2, GL_FLOAT, 0, 0, 0);
		//glVertexAttribPointer(_attUV, 2, GL_FLOAT, 0, 0, UVArray);
		if (addExtraUniform())
			break;

		//draw
		if (m_type == RGBA_I420) {
			glViewport(0, 0, width, height * 3 / 2);
		}
		else {
			glViewport(0, 0, width, height);
		}
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDisableVertexAttribArray(_attPosition);
		glDisableVertexAttribArray(_attUV);
		glDeleteBuffers(2, _vboIds);
		glDeleteVertexArrays(1, &_vao);
		ret = glGetError();
		//glFlush();
		ret = 0;
	} while (false);
	//恢复原fbo
	glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);

	ret = glGetError();
	return ret;
}

int BEFColorConverter::setParam(const char* key, int value) {
	if (m_param.find(key) == m_param.end()) {
		return -1;
	}
	m_param[key] = value;
	return 0;
}

//////////////////////////////////////////
////////////BGRA_RGBAConverter////////////
//////////////////////////////////////////

const char* BGRA_RGBAConverter::getFragmentShader() {
	return R"(
varying vec2 textureCoordinate;
uniform sampler2D u_inputTexture;

void main()
{
    vec4 color = texture2D(u_inputTexture, textureCoordinate);
    gl_FragColor = vec4(color.b, color.g, color.r, color.a);
}
)";
}

//////////////////////////////////////////
///////////YUV_RGBAConverter/////////////
//////////////////////////////////////////
YUV_RGBAConverter::YUV_RGBAConverter() {
	m_param.insert(std::make_pair("videoRange", VIDEO_RANGE::FULL));
	m_param.insert(std::make_pair("colorSpace", COLOR_SPACE::SD_REC601));
}

static const float YUV_RGBA_MAT601[] = {
	1.164384f,  0.000000f,  1.596027f,  -0.874202f,
	1.164384f,  -0.391762f, -0.812968f, 0.531668f,
	1.164384f,  2.017232f,  0.000000f,  -1.085631f,
	0.000000f,  0.000000f,  0.000000f,  1.000000f
};

static const float YUV_RGBA_MAT709[] = {
	1.164384f,  0.000000f,  1.792741f,  -0.972945f,
	1.164384f,  -0.213249f, -0.532909f,  0.301483f,
	1.164384f,  2.112402f,  0.000000f,  -1.133402f,
	0.000000f,  0.000000f,  0.000000f,  1.000000f
};

static const float YUV_RGBA_IDENTITY[] = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};

static const float RGBA_YUV_MAT601[] = {
	0.257f,     0.504f,     0.098f,     0.0625,
	-0.148f,    -0.291f,    0.439f,     0.5,
	0.439f,     -0.368f,    -0.071f,    0.5,
	0.000000f,  0.000000f,  0.000000f,  1.000000f
};

static const float RGBA_YUV_MAT709[] = {
	0.183f,     0.614f,     0.062f,     0.0625f,
	-0.101f,    -0.339f,    0.439f,     0.5f,
	0.439f,     -0.399f,    -0.040f,    0.5f,
	0.000000f,  0.000000f,  0.000000f,  1.000000f
};

static const float YUV_RGBA_FULL_MIN[] = { 0.0f, 0.0f, 0.0f, 1.0f };
static const float YUV_RGBA_FULL_MAX[] = { 1.0f, 1.0f, 1.0f, 1.0f };

static const float YUV_RGBA_PARTIAL_MIN[] = { 0.0627451017f, 0.0627451017f, 0.0627451017f, 1.0f };
static const float YUV_RGBA_PARTIAL_MAX[] = { 0.921568632f, 0.941176474f, 0.941176474f, 1.0f };

int YUV_RGBAConverter::addExtraUniform() {
	glUseProgram(m_effectGLEnv->m_program);

	GLint _rangeMin = glGetUniformLocation(m_effectGLEnv->m_program, "u_rangeMin");
	if (-1 == _rangeMin)
		return -1;
	GLint _rangeMax = glGetUniformLocation(m_effectGLEnv->m_program, "u_rangeMax");
	if (-1 == _rangeMax)
		return -1;
	GLint _yuvMat = glGetUniformLocation(m_effectGLEnv->m_program, "u_yuvMat");
	if (-1 == _yuvMat)
		return -1;
	GLint _outputSize = glGetUniformLocation(m_effectGLEnv->m_program, "u_outputSize");
	if (-1 == _outputSize)
		return -1;

	switch (m_param["videoRange"]) {
	case VIDEO_RANGE::PARTIAL:
		glUniform4fv(_rangeMin, 1, YUV_RGBA_PARTIAL_MIN);
		glUniform4fv(_rangeMax, 1, YUV_RGBA_PARTIAL_MAX);
		break;
	case VIDEO_RANGE::FULL:
	default:
		glUniform4fv(_rangeMin, 1, YUV_RGBA_FULL_MIN);
		glUniform4fv(_rangeMax, 1, YUV_RGBA_FULL_MAX);
		break;
	}
	switch (m_param["colorSpace"]) {
	case COLOR_SPACE::HD_REC709:
		glUniformMatrix4fv(_yuvMat, 1, GL_FALSE, YUV_RGBA_MAT709);
		break;
	case COLOR_SPACE::SD_REC601:
		glUniformMatrix4fv(_yuvMat, 1, GL_FALSE, YUV_RGBA_MAT601);
		break;
	case COLOR_SPACE::JPEG:
	default:
		glUniformMatrix4fv(_yuvMat, 1, GL_FALSE, YUV_RGBA_IDENTITY);
		break;
	}

	float size[] = { float(m_effectGLEnv->m_outputWidth), float(m_effectGLEnv->m_outputHeight) };
	glUniform2fv(_outputSize, 1, size);

	return 0;
}


//////////////////////////////////////////
///////////YUY2_RGBAConverter/////////////
//////////////////////////////////////////

const char* YUY2_RGBAConverter::getFragmentShader() {
	return R"(
varying vec2      textureCoordinate; // 0.05, 0.15, 0.25, 0.35, 0.45, 0.55, 0.65, 0.75, 0.85, 0.95
uniform sampler2D u_inputTexture;
uniform vec4      u_rangeMin;
uniform vec4      u_rangeMax;
uniform mat4      u_yuvMat;
uniform vec2      u_outputSize; // (10, y)

void main()
{
    float cx = 1.0 / u_outputSize.x; // 0.1
    float odd = floor(mod(textureCoordinate.x * u_outputSize.x, 2.0)); // 0, 1, 0, 1, 0, 1, 0, 1, 0, 1
    float x = textureCoordinate.x + 0.5 * cx - odd * cx; // 0.1, 0.1, 0.3, 0.3, 0.5, 0.5, 0.7, 0.7, 0.9, 0.9
    vec4 color = texture2D(u_inputTexture, vec2(x, textureCoordinate.y));
    color = vec4(odd < 0.5 ? color[0] : color[2], color[1], color[3], 1.0);
    color = clamp(color, u_rangeMin, u_rangeMax);
    color = color * u_yuvMat;
    gl_FragColor = color;
}
)";
}

//////////////////////////////////////////
////////////I420_RGBAConverter////////////
//////////////////////////////////////////

const char* I420_RGBAConverter::getFragmentShader() {
	return R"(
varying vec2      textureCoordinate; // 0.05, 0.15, 0.25, 0.35, 0.45, 0.55, 0.65, 0.75, 0.85, 0.95
uniform sampler2D u_inputTexture;
uniform vec4      u_rangeMin;
uniform vec4      u_rangeMax;
uniform mat4      u_yuvMat;
uniform vec2      u_outputSize; // (10, y)

void main()
{
    float odd = floor(mod(textureCoordinate.y * u_outputSize.y, 4.0));
    vec4 color;
    color[0] = texture2D(u_inputTexture, vec2(textureCoordinate.x, textureCoordinate.y * 2.0 / 3.0)).r;
    float x = odd < 0.5 ? (textureCoordinate.x / 2.0) : (textureCoordinate.x / 2.0 + 0.5);
    color[1] = texture2D(u_inputTexture, vec2(x, 2.0 / 3.0 + textureCoordinate.y * 1.0 / 6.0)).r;
    color[2] = texture2D(u_inputTexture, vec2(x, 5.0 / 6.0 + textureCoordinate.y * 1.0 / 6.0)).r;
    color[3] = 1.0;
    vec4 outColor = clamp(color, u_rangeMin, u_rangeMax);
    outColor = color * u_yuvMat;
    gl_FragColor = outColor;
}
)";
}

//////////////////////////////////////////
////////////RGBA_BGRAConverter////////////
//////////////////////////////////////////

const char* RGBA_BGRAConverter::getFragmentShader() {
	return R"(
varying vec2 textureCoordinate;
uniform sampler2D u_inputTexture;

void main()
{
    vec4 color = texture2D(u_inputTexture, textureCoordinate);
    gl_FragColor = vec4(color.b, color.g, color.r, color.a);
}
)";
}

//////////////////////////////////////////
////////////RGBA_I420Converter////////////
//////////////////////////////////////////

int RGBA_I420Converter::addExtraUniform() {
	YUV_RGBAConverter::addExtraUniform();

	glUseProgram(m_effectGLEnv->m_program);

	GLint _yuvMat = glGetUniformLocation(m_effectGLEnv->m_program, "u_yuvMat");
	if (-1 == _yuvMat)
		return -1;
	GLint _outputSize = glGetUniformLocation(m_effectGLEnv->m_program, "u_outputSize");
	if (-1 == _outputSize)
		return -1;

	switch (m_param["colorSpace"]) {
	case COLOR_SPACE::HD_REC709:
		glUniformMatrix4fv(_yuvMat, 1, GL_FALSE, RGBA_YUV_MAT709);
		break;
	case COLOR_SPACE::SD_REC601:
		glUniformMatrix4fv(_yuvMat, 1, GL_FALSE, RGBA_YUV_MAT601);
		break;
	case COLOR_SPACE::JPEG:
	default:
		glUniformMatrix4fv(_yuvMat, 1, GL_FALSE, YUV_RGBA_IDENTITY);
		break;
	}

	float size[] = { float(m_effectGLEnv->m_outputWidth), float(m_effectGLEnv->m_outputHeight) };
	glUniform2fv(_outputSize, 1, size);

	return 0;
}

const char* RGBA_I420Converter::getFragmentShader() {
	return R"(
varying vec2      textureCoordinate; // 0.05, 0.15, 0.25, 0.35, 0.45, 0.55, 0.65, 0.75, 0.85, 0.95
uniform sampler2D u_inputTexture;
uniform vec4      u_rangeMin;
uniform vec4      u_rangeMax;
uniform mat4      u_yuvMat;
uniform vec2      u_outputSize; // (10, y)

vec4 RGBAtoRGB(vec4 color) {
    float alpha = color.a;
    vec4 newColor;
    newColor.a = 1.0;
    newColor.r = 1.0 - alpha + alpha*color.r;
    newColor.g = 1.0 - alpha + alpha*color.g;
    newColor.b = 1.0 - alpha + alpha*color.b;

    return newColor;
}

void main()
{
    if (textureCoordinate.y < 2.0 / 3.0) { //Y
        vec2 coord;
        coord.x = textureCoordinate.x;
        coord.y = textureCoordinate.y * 3.0 / 2.0;
        vec4 srcColor = texture2D(u_inputTexture, coord);
        srcColor = RGBAtoRGB(srcColor);
        vec4 yuvValue = srcColor * u_yuvMat;
        float yValue = yuvValue[0];
        gl_FragColor = vec4(yValue);
    }
    else {
        if (textureCoordinate.y < 5.0 / 6.0) { //U
            float odd = textureCoordinate.x < 0.5 ? 0.0 : 1.0;
            vec2 coord;
            coord.x = (textureCoordinate.x - odd * 0.5) * 2.0;
            coord.y = (textureCoordinate.y - 2.0 / 3.0) * 6.0 + odd / u_outputSize.y;
            vec4 srcColor = texture2D(u_inputTexture, coord);
            srcColor = RGBAtoRGB(srcColor);
            vec4 yuvValue = srcColor * u_yuvMat;
            float uValue = yuvValue[1];
            gl_FragColor = vec4(uValue);
        }
    }
    if (textureCoordinate.y >= 5.0 / 6.0){ //V
        float odd = textureCoordinate.x < 0.5 ? 0.0 : 1.0;
        vec2 coord;
        coord.x = (textureCoordinate.x - odd * 0.5) * 2.0;
        coord.y = (textureCoordinate.y - 5.0 / 6.0) * 6.0 + odd / u_outputSize.y;
        vec4 srcColor = texture2D(u_inputTexture, coord);
        srcColor = RGBAtoRGB(srcColor);
        vec4 yuvValue = srcColor * u_yuvMat;
        float vValue = yuvValue[2];
        gl_FragColor = vec4(vValue);
    }
}
)";
}

//////////////////////////////////////////
////////////I420_RGBA_3C_Converter////////////
//////////////////////////////////////////

const char* I420_RGBA_3C_Converter::getFragmentShader() {
	return R"(
varying vec2      textureCoordinate; // 0.05, 0.15, 0.25, 0.35, 0.45, 0.55, 0.65, 0.75, 0.85, 0.95
uniform sampler2D u_inputY;
uniform sampler2D u_inputU;
uniform sampler2D u_inputV;
uniform vec4      u_rangeMin;
uniform vec4      u_rangeMax;
uniform mat4      u_yuvMat;
uniform vec2      u_outputSize; // (10, y)

void main()
{
    float odd = floor(mod(textureCoordinate.y * u_outputSize.y, 4.0));
    vec4 color;
    color[0] = texture2D(u_inputY, textureCoordinate).a;
    color[1] = texture2D(u_inputU, textureCoordinate).a;
    color[2] = texture2D(u_inputV, textureCoordinate).a;
    color[3] = 1.0;
    vec4 outColor = clamp(color, u_rangeMin, u_rangeMax);
    outColor = color * u_yuvMat;
    gl_FragColor = outColor;
}
)";
}

int I420_RGBA_3C_Converter::convert(unsigned int srcY, unsigned int srcU, unsigned int srcB, unsigned int dstTexture, int width, int height) {
	int ret = -1;
	if (width <= 0 || height <= 0) {
		return -1;
	}
	m_effectGLEnv->m_outputWidth = width;
	m_effectGLEnv->m_outputHeight = height;
	//保存原fbo
	GLuint prevFbo = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&prevFbo);
	do {
		if (dstTexture != 0) {
			glBindFramebuffer(GL_FRAMEBUFFER, m_effectGLEnv->m_fbo);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dstTexture, 0);
		}
		else {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		glClearColor(1.0, 0.0, 0.0, 1.0);

		GLint _attPosition = glGetAttribLocation(m_effectGLEnv->m_program, "attPosition");
		if (-1 == _attPosition)
			break;
		GLint _attUV = glGetAttribLocation(m_effectGLEnv->m_program, "attUV");
		if (-1 == _attUV)
			break;
		GLint _inputY = glGetUniformLocation(m_effectGLEnv->m_program, "u_inputY");
		if (-1 == _inputY)
			break;
		GLint _inputU = glGetUniformLocation(m_effectGLEnv->m_program, "u_inputU");
		if (-1 == _inputU)
			break;
		GLint _inputV = glGetUniformLocation(m_effectGLEnv->m_program, "u_inputV");
		if (-1 == _inputV)
			break;
		GLint _flipX = glGetUniformLocation(m_effectGLEnv->m_program, "u_flipX");
		if (-1 == _flipX)
			break;
		GLint _flipY = glGetUniformLocation(m_effectGLEnv->m_program, "u_flipY");
		if (-1 == _flipY)
			break;

		//prepareProgram
		glUseProgram(m_effectGLEnv->m_program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, srcY);
		glUniform1i(_inputY, 0);
		glBindTexture(GL_TEXTURE_2D, srcU);
		glUniform1i(_inputU, 1);
		glBindTexture(GL_TEXTURE_2D, srcB);
		glUniform1i(_inputV, 2);
		if (m_param["flipX"]) {
			glUniform1f(_flipX, -1);
		}
		else {
			glUniform1f(_flipX, 1);
		}
		if (m_param["flipY"]) {
			glUniform1f(_flipY, -1);
		}
		else {
			glUniform1f(_flipY, 1);
		}
		GLuint _vao;
		glGenVertexArrays(1, &_vao);
		glBindVertexArray(_vao);
		GLuint _vboIds[2];;
		glGenBuffers(2, _vboIds);
		glBindBuffer(GL_ARRAY_BUFFER, _vboIds[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(VertexArray), VertexArray, GL_STATIC_DRAW);
		glEnableVertexAttribArray(_attPosition);
		//glVertexAttribPointer(_attPosition, 3, GL_FLOAT, 0, 0, VertexArray);
		glVertexAttribPointer(_attPosition, 3, GL_FLOAT, 0, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, _vboIds[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(UVArray), UVArray, GL_STATIC_DRAW);
		glEnableVertexAttribArray(_attUV);
		glVertexAttribPointer(_attUV, 2, GL_FLOAT, 0, 0, 0);
		//glVertexAttribPointer(_attUV, 2, GL_FLOAT, 0, 0, UVArray);
		if (addExtraUniform())
			break;

		//draw
		if (m_type == RGBA_I420) {
			glViewport(0, 0, width, height * 3 / 2);
		}
		else {
			glViewport(0, 0, width, height);
		}
		ret = glGetError();
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDisableVertexAttribArray(_attPosition);
		glDisableVertexAttribArray(_attUV);
		glDeleteBuffers(2, _vboIds);
		glDeleteVertexArrays(1, &_vao);
		ret = glGetError();
		//glFlush();
		ret = 0;
	} while (false);
	//恢复原fbo
	glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);

	ret = glGetError();
	return ret;
}



BEFColorConverter* BEFColorConverterFactory::create(bef_color_type inputType, bef_color_type outputType) {
	if (outputType == color_rgba) {
		switch (inputType) {
		case color_bgra:
		case color_bgr:
			return new BGRA_RGBAConverter();
		case color_yuy2:
			return new YUY2_RGBAConverter();
		case color_i420:
			return new I420_RGBAConverter();
		default:
			return new BEFColorConverter();
		}
	}
	else if (inputType == color_rgba) {
		switch (outputType) {
		case color_i420:
			return new RGBA_I420Converter();
		case color_bgra:
		case color_bgr:
			return new RGBA_BGRAConverter();
		default:
			return new BEFColorConverter();
		}
	}
	return nullptr;
}

NAMESPACE_BEF_EFFECT_FRAMEWORK_END
