#include "BEImageUtil.h"

BEConvertProgram::BEConvertProgram() {
	m_convert = nullptr;
}

BEConvertProgram::~BEConvertProgram() {
	destroy();
}

void BEConvertProgram::init(bef_color_type inputType, bef_color_type outputType) {
	m_convert = BEF::BEFColorConverterFactory::create(inputType, outputType);
	m_convert->init();

	m_colorType = outputType;
	m_texture = m_convert->createTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
}

void BEConvertProgram::destroy() {
	if (m_convert) {
		m_convert->destroy();
		delete m_convert;
		m_convert = nullptr;
	}

	if (m_texture != 0) {
		glDeleteTextures(1, &m_texture);
		m_texture = 0;
	}
}

int BEConvertProgram::transfer(int inputTexture, int width, int height) {
	m_convert->updateTexture(m_texture, nullptr, width, height, m_colorType);

	m_convert->convert(inputTexture, m_texture, width, height);

	return m_texture;
}

BEF::BEFColorConverter* BEConvertProgram::getConvert() {
	return m_convert;
}

/// ///////////////////////////////////////////////

BEImageUtil::BEImageUtil() {

}

BEImageUtil::~BEImageUtil() {

}

void BEImageUtil::init() {
	glGenFramebuffers(1, &m_fbo);
}

void BEImageUtil::destroy() {
	if (m_BGRX2RGBAConvert != nullptr) {
		delete m_BGRX2RGBAConvert;
		m_BGRX2RGBAConvert = nullptr;
	}

	if (m_I4202RGBAConvert != nullptr) {
		delete m_I4202RGBAConvert;
		m_I4202RGBAConvert = nullptr;
	}

	if (m_RGBA2I420Convert != nullptr) {
		delete m_RGBA2I420Convert;
		m_RGBA2I420Convert = nullptr;
	}

	if (m_RGBA2BGRXConvert != nullptr) {
		delete m_RGBA2BGRXConvert;
		m_RGBA2BGRXConvert = nullptr;
	}

	glDeleteFramebuffers(1, &m_fbo);
}


unsigned int BEImageUtil::createTexture() {
	unsigned int textureID = 0;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	return textureID;
}

void BEImageUtil::transferBufferToTexture(unsigned char* buffer, int width, int height, int textureId, bef_color_type inputFormat) {
	GLenum m_format = GL_RGBA;
	int inputWidth = width;
	int inputHeight = height;
	switch (inputFormat) {
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
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, m_format, inputWidth, inputHeight, 0, m_format, GL_UNSIGNED_BYTE, buffer);
	GLenum ret = glGetError();
	glBindTexture(GL_TEXTURE_2D, 0);
	return;
}

int BEImageUtil::transferTextureToTexture(int inputTexture, bef_color_type inputFormat, bef_color_type outputFormat, int width, int height) {
	if (outputFormat == color_rgba) {
		if (inputFormat == color_bgra ||
			inputFormat == color_bgr) {
			return getBGRX2RGBAConvert()->transfer(inputTexture, width, height);
		}
		else if (inputFormat == color_i420) {
			return getI4202RGBAConvert()->transfer(inputTexture, width, height);
		}
	}
	else {
		if (inputFormat == color_rgba) {
			if (outputFormat == color_i420) {
				return getRGBA2I420Convert()->transfer(inputTexture, width, height);
			}
			else if (outputFormat == color_bgra ||
				outputFormat == color_bgr) {
				return getRGBA2BGRXConvert()->transfer(inputTexture, width, height);
			}
		}
	}

	return -1;
}

void BEImageUtil::readPixel(unsigned int textureID, void* buffer, int width, int height, bef_color_type format) {
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
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
	glViewport(0, 0, width, height);
	glReadPixels(0, 0, width, height, m_format, GL_UNSIGNED_BYTE, buffer);
	//恢复原fbo
	glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);

	return;
}

void BEImageUtil::RGBA2RGB(unsigned char* src, unsigned char* dst, int width, int height) {
	unsigned int* rbgaPtr = (unsigned int*)src;
	for (int i = 0; i < height; i++) {
		unsigned char* linePtr = (unsigned char*)(src + i * width * 4);
		unsigned char* dstLinePtr = (unsigned char*)(dst + i * width * 3);
		for (int j = 0; j < width; j++) {
			unsigned char r = linePtr[j * 4 + 0];
			unsigned char g = linePtr[j * 4 + 1];
			unsigned char b = linePtr[j * 4 + 2];
			unsigned char a = linePtr[j * 4 + 3];

			float alpha = (float)a / 255.0;

			dstLinePtr[j * 3] = (1.0 - alpha) * 255 + alpha * r;       // red
			dstLinePtr[j * 3 + 1] = (1.0 - alpha) * 255 + alpha * g;   // green
			dstLinePtr[j * 3 + 2] = (1.0 - alpha) * 255 + alpha * b;   // blue
		}
	}
}

BEConvertProgram* BEImageUtil::getBGRX2RGBAConvert() {
	if (m_BGRX2RGBAConvert == nullptr) {
		m_BGRX2RGBAConvert = new BEConvertProgram();
		m_BGRX2RGBAConvert->init(BEF::color_bgra, BEF::color_rgba);
	}
	return m_BGRX2RGBAConvert;
}

BEConvertProgram* BEImageUtil::getI4202RGBAConvert() {
	if (m_I4202RGBAConvert == nullptr) {
		m_I4202RGBAConvert = new BEConvertProgram();
		m_I4202RGBAConvert->init(BEF::color_i420, BEF::color_rgba);
	}
	return m_I4202RGBAConvert;
}

BEConvertProgram* BEImageUtil::getRGBA2I420Convert() {
	if (m_RGBA2I420Convert == nullptr) {
		m_RGBA2I420Convert = new BEConvertProgram();
		m_RGBA2I420Convert->init(BEF::color_rgba, BEF::color_i420);
	}
	return m_RGBA2I420Convert;
}

BEConvertProgram* BEImageUtil::getRGBA2BGRXConvert() {
	if (m_RGBA2BGRXConvert == nullptr) {
		m_RGBA2BGRXConvert = new BEConvertProgram();
		m_RGBA2BGRXConvert->init(BEF::color_rgba, BEF::color_bgra);
	}
	return m_RGBA2BGRXConvert;
}