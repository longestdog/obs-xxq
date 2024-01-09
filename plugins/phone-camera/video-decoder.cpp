#include "video-decoder.h"
#include "frame-renderer.h"
#include <util/base.h>
#include <obs.h>

static AVPixelFormat get_dxva2_hw_format(AVCodecContext *avctx, const enum AVPixelFormat *pix_fmts)
{
	while (*pix_fmts != AV_PIX_FMT_NONE) {
		if (*pix_fmts == AV_PIX_FMT_DXVA2_VLD) {
			AVBufferRef *hw_device_ref = (AVBufferRef *)avctx->opaque;
			AVHWFramesContext *frames_ctx = nullptr;
			AVDXVA2FramesContext *frames_hwctx = nullptr;
			avctx->hw_frames_ctx = av_hwframe_ctx_alloc(hw_device_ref);
			if (!avctx->hw_frames_ctx) {
				return AV_PIX_FMT_NONE;
			}

			frames_ctx = (AVHWFramesContext *)avctx->hw_frames_ctx->data;
			frames_hwctx = (AVDXVA2FramesContext *)frames_ctx->hwctx;

			frames_ctx->format = AV_PIX_FMT_DXVA2_VLD;
			frames_ctx->sw_format = AV_PIX_FMT_NV12;
			frames_ctx->width = FFALIGN(avctx->coded_width, 32);
			frames_ctx->height = FFALIGN(avctx->coded_height, 32);
			frames_ctx->initial_pool_size = 10;

			int ret = av_hwframe_ctx_init(avctx->hw_frames_ctx);
			if (ret < 0) {
				return AV_PIX_FMT_NONE;
			}

			return AV_PIX_FMT_DXVA2_VLD;
		}

		pix_fmts++;
	}

	blog(LOG_DEBUG, "Failed to get HW surface format.");
	return AV_PIX_FMT_NONE;
}

typedef struct DXVA2DevicePriv {
	HMODULE d3dlib;
	HMODULE dxva2lib;

	HANDLE device_handle;

	IDirect3D9 *d3d9;
	IDirect3DDevice9 *d3d9device;
} DXVA2DevicePriv;

typedef HRESULT WINAPI pCreateDeviceManager9(UINT *, IDirect3DDeviceManager9 **);

static void dxva2_device_free(AVHWDeviceContext *device_context)
{
	AVDXVA2DeviceContext *dxva2_device_context = (AVDXVA2DeviceContext *)device_context->hwctx;
	DXVA2DevicePriv *dxva2_device_priv = (DXVA2DevicePriv *)device_context->user_opaque;

	if (dxva2_device_context->devmgr && dxva2_device_priv->device_handle != INVALID_HANDLE_VALUE) {
		dxva2_device_context->devmgr->CloseDeviceHandle(dxva2_device_priv->device_handle);
	}

	if (dxva2_device_context->devmgr) {
		dxva2_device_context->devmgr->Release();
	}

	if (dxva2_device_priv->d3d9device) {
		dxva2_device_priv->d3d9device->Release();
	}

	if (dxva2_device_priv->d3d9) {
		dxva2_device_priv->d3d9->Release();
	}

	if (dxva2_device_priv->d3dlib) {
		FreeLibrary(dxva2_device_priv->d3dlib);
	}

	if (dxva2_device_priv->dxva2lib) {
		FreeLibrary(dxva2_device_priv->dxva2lib);
	}

	av_freep(&device_context->user_opaque);
}

static int dxva2_device_create2(AVHWDeviceContext *device_context, IDirect3DDevice9Ex *d3d9_device)
{
	AVDXVA2DeviceContext *dxva2_device_context = (AVDXVA2DeviceContext *)device_context->hwctx;
	DXVA2DevicePriv *dxva2_device_priv = nullptr;
	pCreateDeviceManager9 *createDeviceManager = NULL;

	unsigned reset_token = 0;
	HRESULT hr = S_OK;

	dxva2_device_priv = (DXVA2DevicePriv *)av_mallocz(sizeof(*dxva2_device_priv));
	if (!dxva2_device_priv) {
		return AVERROR(ENOMEM);
	}

	device_context->user_opaque = dxva2_device_priv;
	device_context->free = dxva2_device_free;

	dxva2_device_priv->device_handle = INVALID_HANDLE_VALUE;

	dxva2_device_priv->d3dlib = LoadLibraryA("d3d9.dll");
	if (!dxva2_device_priv->d3dlib) {
		av_log(device_context, AV_LOG_ERROR, "Failed to load D3D9 library\n");
		return AVERROR_UNKNOWN;
	}

	dxva2_device_priv->dxva2lib = LoadLibraryA("dxva2.dll");
	if (!dxva2_device_priv->dxva2lib) {
		av_log(device_context, AV_LOG_ERROR, "Failed to load DXVA2 library\n");
		return AVERROR_UNKNOWN;
	}

	createDeviceManager = (pCreateDeviceManager9 *)GetProcAddress(dxva2_device_priv->dxva2lib, "DXVA2CreateDirect3DDeviceManager9");
	if (!createDeviceManager) {
		av_log(device_context, AV_LOG_ERROR, "Failed to locate DXVA2CreateDirect3DDeviceManager9\n");
		return AVERROR_UNKNOWN;
	}

	d3d9_device->AddRef();
	dxva2_device_priv->d3d9device = d3d9_device;

	hr = createDeviceManager(&reset_token, &dxva2_device_context->devmgr);
	if (FAILED(hr)) {
		av_log(device_context, AV_LOG_ERROR, "Failed to create Direct3D device manager\n");
		return AVERROR_UNKNOWN;
	}

	dxva2_device_context->devmgr->ResetDevice(dxva2_device_priv->d3d9device, reset_token);
	if (FAILED(hr)) {
		av_log(device_context, AV_LOG_ERROR, "Failed to bind Direct3D device to device manager\n");
		return AVERROR_UNKNOWN;
	}

	hr = dxva2_device_context->devmgr->OpenDeviceHandle(&dxva2_device_priv->device_handle);
	if (FAILED(hr)) {
		av_log(device_context, AV_LOG_ERROR, "Failed to open device handle\n");
		return AVERROR_UNKNOWN;
	}

	return 0;
}

AVDecoder::AVDecoder()
{
	m_decodedFrame = av_frame_alloc();
	m_frame = (obs_source_frame2 *)bzalloc(sizeof(obs_source_frame2));
	m_frame->range = VIDEO_RANGE_PARTIAL;
	video_format_get_parameters(VIDEO_CS_601, m_frame->range, m_frame->color_matrix, m_frame->color_range_min, m_frame->color_range_max);
}

AVDecoder::~AVDecoder()
{
	destroy();
	bfree(m_frame);
}

bool AVDecoder::spsChanged(uint8_t *data, size_t len)
{
	if (!m_codecCtx)
		return true;

	return m_codecCtx->extradata_size != len || memcmp(data, m_codecCtx->extradata, len) != 0;
}

bool AVDecoder::init(uint8_t *data, size_t len, bool useSoftware)
{
	if (m_codecCtx != nullptr) {
		blog(LOG_DEBUG, "codec was opened.");
		return true;
	}

	if (!useSoftware) {
		if (initHwDecode(data, len)) {
			m_isHwDecoder = true;
			return true;
		}
	}

	if (initSoftDecode(data, len)) {
		m_isHwDecoder = false;
		return true;
	}

	return false;
}

void AVDecoder::destroy()
{
	if (m_codecCtx != nullptr) {
		avcodec_close(m_codecCtx);
		avcodec_free_context(&m_codecCtx);
		m_codecCtx = nullptr;
	}

	if (m_deviceBuffer) {
		av_buffer_unref(&m_deviceBuffer);
		m_deviceBuffer = nullptr;
	}

	av_frame_free(&m_decodedFrame);

	if (m_renderer) {
		delete m_renderer;
		m_renderer = nullptr;
	}
}

int AVDecoder::send(uint8_t *data, size_t len)
{
	if (m_codecCtx == NULL) {
		return -1;
	}

	m_encodedPacket.data = data;
	m_encodedPacket.size = (int)len;
	int ret = avcodec_send_packet(m_codecCtx, &m_encodedPacket);
	return ret;
}

int AVDecoder::recv(bool drop)
{
	int ret = -1;

	if (m_codecCtx == NULL) {
		return ret;
	}

	switch (m_codecCtx->codec_type) {
	case AVMEDIA_TYPE_VIDEO:
		ret = avcodec_receive_frame(m_codecCtx, m_decodedFrame);
		if (ret >= 0) {
			if (m_decoderReorderPTS == -1) {
				m_decodedFrame->pts = m_decodedFrame->best_effort_timestamp;
			} else if (!m_decoderReorderPTS) {
				m_decodedFrame->pts = m_decodedFrame->pkt_dts;
			}
		}
		break;

	default:
		break;
	}

	if (ret == AVERROR_EOF) {
		avcodec_flush_buffers(m_codecCtx);
	}

	if (ret >= 0 && !drop) {
		if (m_renderer)
			m_renderer->renderFrame(m_decodedFrame);
		else {
			auto ffmpeg_to_obs_video_format = [=](enum AVPixelFormat format) {
				switch (format) {
				case AV_PIX_FMT_YUV444P:
					return VIDEO_FORMAT_I444;
				case AV_PIX_FMT_YUV420P:
				case AV_PIX_FMT_YUVJ420P:
					return VIDEO_FORMAT_I420;
				case AV_PIX_FMT_NV12:
					return VIDEO_FORMAT_NV12;
				case AV_PIX_FMT_YUYV422:
					return VIDEO_FORMAT_YUY2;
				case AV_PIX_FMT_UYVY422:
					return VIDEO_FORMAT_UYVY;
				case AV_PIX_FMT_RGBA:
					return VIDEO_FORMAT_RGBA;
				case AV_PIX_FMT_BGRA:
					return VIDEO_FORMAT_BGRA;
				case AV_PIX_FMT_GRAY8:
					return VIDEO_FORMAT_Y800;
				case AV_PIX_FMT_BGR24:
					return VIDEO_FORMAT_BGR3;
				case AV_PIX_FMT_YUV422P:
					return VIDEO_FORMAT_I422;
				case AV_PIX_FMT_YUVA420P:
					return VIDEO_FORMAT_I40A;
				case AV_PIX_FMT_YUVA422P:
					return VIDEO_FORMAT_I42A;
				case AV_PIX_FMT_YUVA444P:
					return VIDEO_FORMAT_YUVA;
				case AV_PIX_FMT_NONE:
				default:
					return VIDEO_FORMAT_NONE;
				}
			};

			m_frame->timestamp = m_decodedFrame->pts;
			m_frame->width = m_decodedFrame->width;
			m_frame->height = m_decodedFrame->height;
			m_frame->format = ffmpeg_to_obs_video_format((AVPixelFormat)m_decodedFrame->format);
			m_frame->flip = false;
			m_frame->flip_h = false;

			m_frame->data[0] = m_decodedFrame->data[0];
			m_frame->data[1] = m_decodedFrame->data[1];
			m_frame->data[2] = m_decodedFrame->data[2];

			m_frame->linesize[0] = m_decodedFrame->linesize[0];
			m_frame->linesize[1] = m_decodedFrame->linesize[1];
			m_frame->linesize[2] = m_decodedFrame->linesize[2];
		}
	}

	return ret;
}

bool AVDecoder::initHwDecode(uint8_t *data, size_t len)
{
	AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!codec) {
		blog(LOG_DEBUG, "decoder(%s) not found.", avcodec_get_name(AV_CODEC_ID_H264));
		return false;
	}

	AVHWDeviceContext *device_context = nullptr;
	AVDXVA2DeviceContext *d3d9_device_context = nullptr;
	AVHWDeviceType hw_type = AV_HWDEVICE_TYPE_DXVA2;

	m_codecCtx = avcodec_alloc_context3(codec);

	m_renderer = new D3D9Renderer;
	m_renderer->init();

	if (m_renderer->getDevice()) {
		m_deviceBuffer = av_hwdevice_ctx_alloc(hw_type);
		device_context = (AVHWDeviceContext *)m_deviceBuffer->data;
		d3d9_device_context = (AVDXVA2DeviceContext *)device_context->hwctx;
		m_codecCtx->opaque = m_deviceBuffer;
		dxva2_device_create2(device_context, (IDirect3DDevice9Ex *)m_renderer->getDevice());
	} else {
		av_hwdevice_ctx_create(&m_deviceBuffer, hw_type, NULL, NULL, 0);
	}

	m_codecCtx->hw_device_ctx = av_buffer_ref(m_deviceBuffer);
	m_codecCtx->get_format = get_dxva2_hw_format;
	m_codecCtx->thread_count = 1;

	m_codecCtx->extradata = (uint8_t *)av_malloc(len);
	m_codecCtx->extradata_size = (int)len;
	memcpy(m_codecCtx->extradata, data, len);
	if (avcodec_open2(m_codecCtx, codec, NULL) != 0) {
		blog(LOG_DEBUG, "Open decoder(%d) failed.", (int)AV_CODEC_ID_H264);
		goto failed;
	}

	return true;

failed:
	if (m_codecCtx) {
		avcodec_free_context(&m_codecCtx);
		m_codecCtx = nullptr;
	}

	if (m_renderer) {
		delete m_renderer;
		m_renderer = nullptr;
	}

	return false;
}

bool AVDecoder::initSoftDecode(uint8_t *data, size_t len)
{
	AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!codec) {
		blog(LOG_DEBUG, "decoder(%s) not found.", avcodec_get_name(AV_CODEC_ID_H264));
		return false;
	}

	m_codecCtx = avcodec_alloc_context3(codec);
	m_codecCtx->extradata = (uint8_t *)av_malloc(len);
	m_codecCtx->extradata_size = (int)len;
	memcpy(m_codecCtx->extradata, data, len);
	if (avcodec_open2(m_codecCtx, codec, NULL) != 0) {
		blog(LOG_DEBUG, "Open decoder(%d) failed.", (int)AV_CODEC_ID_H264);
		goto failed;
	}

	return true;

failed:
	if (m_codecCtx) {
		avcodec_free_context(&m_codecCtx);
		m_codecCtx = nullptr;
	}
	return false;
}
