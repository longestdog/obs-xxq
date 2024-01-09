#pragma once

#include <string>
#include <memory>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libavutil/hwcontext.h"
#include "libavutil/pixdesc.h"
#include <libavcodec/dxva2.h>
#include <libavutil/hwcontext_dxva2.h>
}
#include "frame-renderer.h"

class AVDecoder {
public:
	AVDecoder &operator=(const AVDecoder &) = delete;
	AVDecoder(const AVDecoder &) = delete;
	AVDecoder();
	~AVDecoder();

	bool init(uint8_t *data, size_t len, bool useSoftware);
	void destroy();
	bool spsChanged(uint8_t *data, size_t len);

	virtual int send(uint8_t *data, size_t len);
	virtual int recv(bool drop);

	bool isHWDecode() { return m_isHwDecoder; }

	uint32_t width() { return m_codecCtx ? m_codecCtx->width : 0; }
	uint32_t height() { return m_codecCtx ? m_codecCtx->height : 0; }

	gs_texture_t *texture() { return m_renderer ? m_renderer->outputTexture() : nullptr; }
	struct obs_source_frame2 *frame() { return m_frame; }

private:
	bool initHwDecode(uint8_t *data, size_t len);
	bool initSoftDecode(uint8_t *data, size_t len);

private:
	AVCodecContext *m_codecCtx = nullptr;
	AVDictionary *m_options = nullptr;
	AVBufferRef *m_deviceBuffer = nullptr;
	AVFrame *m_decodedFrame = nullptr;
	AVPacket m_encodedPacket = {0};
	struct obs_source_frame2 *m_frame = nullptr;
	D3D9Renderer *m_renderer = nullptr;

	int m_decoderReorderPTS = -1;
	bool m_isHwDecoder = false;
};
