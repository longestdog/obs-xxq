#ifndef BE_IMAGEUTIL_H
#define BE_IMAGEUTIL_H


#include "BEFColorConverter.h"

using namespace BEF;

class BEConvertProgram {
public:
	BEConvertProgram();
	~BEConvertProgram();

	void init(bef_color_type inputType, bef_color_type outputType);
	void destroy();
	int transfer(int inputTexture, int width, int height);
	BEFColorConverter* getConvert();
private:
	BEFColorConverter* m_convert;
	GLuint m_texture = 0;
	bef_color_type m_colorType;
};

///////////////////////////////////////////////////

class BEImageUtil {
public:
	BEImageUtil();
	~BEImageUtil();

public:
	void init();
	void destroy();
	unsigned int createTexture();
	void transferBufferToTexture(unsigned char* buffer, int width, int height, int textureId, bef_color_type inputFormat);
	int transferTextureToTexture(int inputTexture, bef_color_type inputFormat, bef_color_type outputFormat, int width, int height);

	void readPixel(unsigned int textureID, void* buffer, int width, int height, bef_color_type format);
	void RGBA2RGB(unsigned char* src, unsigned char* dst, int width, int height);

private:
	BEConvertProgram* getBGRX2RGBAConvert();
	BEConvertProgram* getI4202RGBAConvert();
	BEConvertProgram* getRGBA2I420Convert();
	BEConvertProgram* getRGBA2BGRXConvert();

private:
	// for rgb rgba bgr bgra convert
	BEConvertProgram* m_BGRX2RGBAConvert = nullptr;
	BEConvertProgram* m_I4202RGBAConvert = nullptr;
	BEConvertProgram* m_RGBA2I420Convert = nullptr;
	BEConvertProgram* m_RGBA2BGRXConvert = nullptr;

	GLuint m_fbo;
};



#endif
