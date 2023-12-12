#include "EffectHandle.h"
#include "be_util.h"
#include "CommonFuc.h"
#include "RenderUserData.h"
#include <set>

namespace BYTED_EFFECT {

#define DEFAULT_TEXTURE_WIDTH 1280
#define DEFAULT_TEXTURE_HEIGHT 720

EffectHandle::EffectHandle() {}

EffectHandle::~EffectHandle() {}

void EffectHandle::release()
{
	stop();
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND: {
	} break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	} break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

bool RenderMsgDelegateManagerCallback(void *observer, unsigned int uMsgId, int arg1, int arg2, const char *arg3)
{
	if (observer != nullptr) {
		EffectHandle *handlePtr = (EffectHandle *)observer;
		handlePtr->msgProc(uMsgId, arg1, arg2, arg3);
	}
	return true;
}

void EffectHandle::createContextWnd()
{
	// Register the window class.
	const wchar_t CLASS_NAME[] = L"BEContext Window Class";

	WNDCLASS wc = {};

	HINSTANCE hInstance = GetModuleHandle(0);
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	// Create the window.

	HWND hwnd = CreateWindowEx(0,                   // Optional window styles.
				   CLASS_NAME,          // Window class
				   L"",                 // Window text
				   WS_OVERLAPPEDWINDOW, // Window style
				   0, 0, 10, 10,
				   NULL,      // Parent window
				   NULL,      // Menu
				   hInstance, // Instance handle
				   NULL       // Additional application data
	);

	if (hwnd == NULL) {
		return;
	}
	m_hContextWnd = hwnd;
}

void EffectHandle::init()
{
#if MULTI_HANDLE_USE
	createContextWnd();
	m_effectGlContext.initGLContext((BEF::BEFEffectGLContext::BEFWindow)m_hContextWnd);
#else
	m_effectGlContext.initGLContext();
#endif
	m_effectGlContext.makeCurrentContext();

	m_width = DEFAULT_TEXTURE_WIDTH;
	m_height = DEFAULT_TEXTURE_HEIGHT;

	int ret = bef_effect_ai_create(&m_effectHandle);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "EffectHandle::initializeHandle:: create effect handle failed !");

	ret = bef_effect_ai_check_license(m_effectHandle, m_licensePath.c_str());
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "EffectHandle::initializeHandle:: check_license failed !");

	ret = bef_effect_ai_set_render_api(m_effectHandle, bef_ai_render_api_gles30);

	ret = bef_effect_ai_init(m_effectHandle, m_width, m_height, m_modelDir.c_str(), "");
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "EffectHandle::initializeHandle:: init effect handle failed !");

	ret = bef_effect_ai_composer_set_mode(m_effectHandle, 1, 0);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "EffectHandle::initializeHandle:: bef_effect_ai_composer_set_mode failed !");

#if EFFECT_MESSAGE_USE
	bef_render_msg_delegate_manager_init(&m_renderMsgManager);
#endif
	m_imageUtil = new BEImageUtil();
	m_imageUtil->init();

	m_inputTexture = m_imageUtil->createTexture();
	m_outputTexture = m_imageUtil->createTexture();

	m_format = BEF::color_rgba;

	m_output.buffer = new unsigned char[m_width * m_height * 4];
	m_readBuffer = new unsigned char[m_width * m_height * 4];

	m_bInit = true;
	m_effectGlContext.doneCurrentContext();
}

void EffectHandle::render(void *userData)
{
	m_effectGlContext.makeCurrentContext();

	if (userData != nullptr) {
		RenderUserDataBase *data = (RenderUserDataBase *)userData;
		if (data->dataType() == render_user_data_loadresource) {
			bef_effect_ai_load_resource_with_timeout(m_effectHandle, -1);
			return;
		}
	}

	if (m_input.width != m_width || m_input.height != m_height) {
		m_width = m_input.width;
		m_height = m_input.height;
		if (m_output.buffer != nullptr) {
			delete[] m_output.buffer;
			m_output.buffer = nullptr;
		}
		if (m_readBuffer != nullptr) {
			delete[] m_readBuffer;
			m_readBuffer = nullptr;
		}
		m_output.buffer = new unsigned char[m_width * m_height * 4];
		m_readBuffer = new unsigned char[m_width * m_height * 4];
		// reset pboReader
		if (m_pboReader != nullptr) {
			m_pboReader->release();
			delete m_pboReader;
			m_pboReader = new PBOReader(m_width * m_height * 4);
		}
	}

	GLuint inputTexture = m_inputTexture;
	if (m_format == BEF::color_rgba || m_format == BEF::color_rgb) {
		m_imageUtil->transferBufferToTexture(m_input.buffer, m_input.width, m_input.height, m_inputTexture, m_format);
	} else {
		m_imageUtil->transferBufferToTexture(m_input.buffer, m_input.width, m_input.height, m_inputTexture, m_format);
		inputTexture = m_imageUtil->transferTextureToTexture(m_inputTexture, m_format, BEF::color_rgba, m_input.width, m_input.height);
	}

	m_imageUtil->transferBufferToTexture(nullptr, m_width, m_height, m_outputTexture, BEF::color_rgba);

	auto starTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	double timestamp = (double)starTime.count() / 1000.0;

	START_ALG_TIMER("effect_process");
	bef_effect_result_t ret = bef_effect_ai_set_width_height(m_effectHandle, m_width, m_height);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_set_width_height");
	bef_effect_ai_algorithm_texture(m_effectHandle, inputTexture, timestamp);
	m_renderMtx.lock();
	ret = bef_effect_ai_process_texture(m_effectHandle, inputTexture, m_outputTexture, timestamp);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_process_texture");
	STOP_ALG_TIMER();
	auto endTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

	// 用于性能数据统计
	m_lastRenderTime = endTime.count() - starTime.count();

	if (m_renderCallback) {
		BEFrameInfo frame;
		frame.pixelFormat = (int)BEF_AI_PIX_FMT_RGBA8888;
		frame.oritation = 0;
		frame.frame.width = m_input.width;
		frame.frame.height = m_input.height;
		frame.frame.channel = 4;
		frame.frame.imageData = m_input.buffer;
		m_renderCallback->renderFinish(&frame, m_outputTexture);
	}
	m_renderMtx.unlock();

	m_output.width = m_width;
	m_output.height = m_height;

	if (m_usePBO) {
		pboReadPixel();
	} else {
		if (m_format == BEF::color_bgra || m_format == BEF::color_bgr) {
			int textureOut =
				m_imageUtil->transferTextureToTexture(m_outputTexture, BEF::bef_color_type::color_rgba, BEF::color_bgra, m_width, m_height);
			memset(m_readBuffer, 0, m_width * m_height * 4);

			m_imageUtil->readPixel(textureOut, m_readBuffer, m_width, m_height, m_format);
			if (m_format == BEF::color_bgr) {
				m_imageUtil->RGBA2RGB(m_readBuffer, m_output.buffer, m_width, m_height);
			} else {
				memcpy(m_output.buffer, m_readBuffer, m_width * m_height * 4);
			}
		} else if (m_format == BEF::color_i420) {
			int textureOut =
				m_imageUtil->transferTextureToTexture(m_outputTexture, BEF::bef_color_type::color_rgba, BEF::color_i420, m_width, m_height);
			m_imageUtil->readPixel(textureOut, m_output.buffer, m_width, m_height, m_format);
		} else if (m_format == BEF::color_rgb) {
			m_imageUtil->readPixel(m_outputTexture, m_readBuffer, m_width, m_height, m_format);
			m_imageUtil->RGBA2RGB(m_readBuffer, m_output.buffer, m_width, m_height);
		} else {
			m_imageUtil->readPixel(m_outputTexture, m_output.buffer, m_width, m_height, m_format);
		}
	}

	m_effectGlContext.swapBuffers();
	m_effectGlContext.doneCurrentContext();
}

void EffectHandle::destroy()
{
	if (m_bInit) {
		m_effectGlContext.makeCurrentContext();
		bef_effect_ai_destroy(m_effectHandle);
		if (m_inputTexture != 0) {
			glDeleteTextures(1, &m_inputTexture);
		}
		if (m_outputTexture != 0) {
			glDeleteTextures(1, &m_outputTexture);
		}

		if (m_pboReader) {
			delete m_pboReader;
			m_pboReader = nullptr;
		}

		if (m_imageUtil) {
			m_imageUtil->destroy();
			m_imageUtil = nullptr;
		}

		m_effectGlContext.doneCurrentContext();
		m_effectGlContext.releaseGLContext();
		if (::IsWindow(m_hContextWnd)) {
			PostMessage(m_hContextWnd, WM_CLOSE, (WPARAM)0, (LPARAM)0);
		}
	}

	if (m_output.buffer) {
		delete[] m_output.buffer;
		m_output.buffer = nullptr;
	}
	if (m_readBuffer) {
		delete[] m_readBuffer;
		m_readBuffer = nullptr;
	}
}

void EffectHandle::initResource(const char *licensePath, const char *modelDir)
{
	if (licensePath == nullptr || modelDir == nullptr) {
		return;
	}
	m_licensePath = licensePath;
	m_modelDir = modelDir;
	start();
	m_conditionBegin.wait();
}

void EffectHandle::msgProc(unsigned int uMsgId, int arg1, int arg2, const char *arg3)
{
#if EFFECT_MESSAGE_USE
	if (m_renderMsgDelegate) {
		m_renderMsgDelegate->msgProc(uMsgId, arg1, arg2, arg3);
	}
#endif
}

void EffectHandle::setUsePBO(bool v)
{
	if (CommonFunc::isWin7lowerSystem()) {
		return;
	}

	m_usePBO = v;
}

void EffectHandle::pboReadPixel()
{
	// 用pbo的方式读取，读取纹理加速
	if (m_pboReader == nullptr) {
		m_pboReader = new PBOReader(m_width * m_height * 4);
	}

	GLuint outTex = m_outputTexture;
	int texWidth = m_width;
	int texHeight = m_height;
	int channels = 4;

	if (m_format == BEF::color_bgra || m_format == BEF::color_bgr) {
		outTex = m_imageUtil->transferTextureToTexture(m_outputTexture, BEF::color_rgba, BEF::color_bgra, m_width, m_height);
	} else if (m_format == BEF::color_i420) {
		outTex = m_imageUtil->transferTextureToTexture(m_outputTexture, BEF::color_rgba, BEF::color_i420, m_width, m_height);
		texHeight = texHeight * 3 / 2;
		channels = 1;
	} else {
		outTex = m_outputTexture;
	}

	PBOReader::DisposableBuffer buf;
	if (m_pboReader->read(outTex, texWidth, texHeight, channels, getReadPixelFormat(), buf)) {
		//把buf 里的东西拷贝出来，不要直接把buf.get() 传出去，buf对象析构以后buf.get 返回的内容是为定义的。
		if (m_format == BEF::color_bgr || m_format == BEF::color_rgb) {
			m_imageUtil->RGBA2RGB(buf.get(), m_output.buffer, m_width, m_height);
		} else {
			memcpy(m_output.buffer, buf.get(), m_width * m_height * 4);
		}
		if (m_isFirstFrame) {
			// 第一帧双pbo的方式因为第一次纹理交换为空，读取出来是空的图像
			// 第一帧采用readpixel的方式
			m_isFirstFrame = false;
			if (m_format == BEF::color_i420) {
				//m_imageUtil->readPixel(outTex, m_output.buffer, m_width, m_height, m_format);
			} else {
				m_imageUtil->readPixel(outTex, m_readBuffer, m_width, m_height, m_format);

				if (m_format == BEF::color_bgr || m_format == BEF::color_rgb) {
					m_imageUtil->RGBA2RGB(m_readBuffer, m_output.buffer, m_width, m_height);
				}
			}
		}
	}
}

void EffectHandle::setRenderCallback(IRenderCallback *callback)
{
	std::lock_guard<std::mutex> al(m_renderMtx);
	m_renderCallback = callback;
}

void EffectHandle::setMsgDelegate(IRenderMsgDelegate *renderMsgDelegate)
{
#if EFFECT_MESSAGE_USE
	m_renderMsgDelegate = renderMsgDelegate;
	bef_render_msg_delegate_manager_add(m_renderMsgManager, this, RenderMsgDelegateManagerCallback);
#endif
}

void EffectHandle::setLogFunc(logEffectFuncPointer logFunc)
{
	bef_effect_ai_set_log_callback((bef_ai_log_callback)logFunc);
}

void EffectHandle::setInput(int width, int height, unsigned char *imageData, pixel_format_type format)
{
	m_input.width = width;
	m_input.height = height;
	m_input.buffer = imageData;
	//memcpy(m_input.buffer, imageData, width*height*4);
	switch (format) {
	case pixel_format_rgba:
		m_format = BEF::color_rgba;
		break;
	case pixel_format_rgb:
		m_format = BEF::color_rgb;
		break;
	case pixel_format_bgra:
		m_format = BEF::color_bgra;
		break;
	case pixel_format_bgr:
		m_format = BEF::color_bgr;
		break;
	case pixel_format_yuy2:
		m_format = BEF::color_yuy2;
		break;
	case pixel_format_i420:
		m_format = BEF::color_i420;
		break;
	default:
		break;
	}
	commit(m_input);
}

unsigned char *EffectHandle::getOutput()
{
	return m_output.buffer;
}

void EffectHandle::appendComposerNodes(const std::vector<std::string> &nodes, const std::vector<std::string> &tags)
{
	std::lock_guard<std::mutex> al(m_renderMtx);
	if (nodes.size() == 0) {
		return;
	}
	char **nodesPath = (char **)malloc(nodes.size() * sizeof(char *));
	char **nodeTags = nullptr;
	if (tags.size() != 0) {
		nodeTags = (char **)malloc(nodes.size() * sizeof(char *));
	}

	std::set<std::string> nodeSet;
	int count = 0;
	for (int i = 0; i < nodes.size(); ++i) {
		const std::string &node = nodes[i];

		if (nodeSet.find(node) != nodeSet.end()) {
			//去除重复的节点
			continue;
		}

		nodeSet.insert(node);

		if (!nodes[i].empty()) {
			nodesPath[count] = (char *)nodes[i].c_str();
		}

		if (tags.size() != 0) {
			nodeTags[count] = (char *)tags[i].c_str();
		}

		count++;
	}

	bef_effect_result_t ret = BEF_RESULT_SUC;
	if (tags.size() <= 0) {
		ret = bef_effect_ai_composer_append_nodes(m_effectHandle, (const char **)nodesPath, count);
	} else {
		ret = bef_effect_ai_composer_append_nodes_with_tags(m_effectHandle, (const char **)nodesPath, (const char **)nodeTags, count);
	}
	if (ret != BEF_RESULT_SUC) {
		printf("bef_effect_ai_composer_append_nodes error:%d\n", ret);
	}
	if (nodesPath != nullptr) {
		free(nodesPath);
	}
	if (nodeTags != nullptr) {
		free(nodeTags);
	}
}

void EffectHandle::updateComposerNodes(const std::vector<std::string> &nodes, const std::vector<std::string> &tags)
{
	std::lock_guard<std::mutex> al(m_renderMtx);
	if (nodes.size() == 0) {
		bef_effect_ai_composer_set_nodes(m_effectHandle, nullptr, 0);
		return;
	}
	char **nodesPath = (char **)malloc(nodes.size() * sizeof(char *));
	char **nodeTags = nullptr;
	if (tags.size() != 0) {
		nodeTags = (char **)malloc(nodes.size() * sizeof(char *));
	}

	std::set<std::string> nodeSet;
	int count = 0;
	for (int i = 0; i < nodes.size(); ++i) {
		const std::string &node = nodes[i];

		if (nodeSet.find(node) != nodeSet.end()) {
			//去除重复的节点
			continue;
		}

		nodeSet.insert(node);

		if (!nodes[i].empty()) {
			nodesPath[count] = (char *)nodes[i].c_str();
		}

		if (tags.size() != 0) {
			nodeTags[count] = (char *)tags[i].c_str();
		}

		count++;
	}

	bef_effect_result_t ret = BEF_RESULT_SUC;
	if (tags.size() <= 0) {
		ret = bef_effect_ai_composer_set_nodes(m_effectHandle, (const char **)nodesPath, count);
	} else {
		ret = bef_effect_ai_composer_set_nodes_with_tags(m_effectHandle, (const char **)nodesPath, (const char **)nodeTags, count);
	}

	if (ret != BEF_RESULT_SUC) {
		printf("bef_effect_ai_composer_set_nodes error:%d\n", ret);
	}

	if (nodesPath != nullptr) {
		free(nodesPath);
	}
	if (nodeTags != nullptr) {
		free(nodeTags);
	}

	setNeedLoadResource(true);
}

void EffectHandle::updateComposerNodeIntensity(const char *node, const char *key, float intensity)
{
	std::lock_guard<std::mutex> al(m_renderMtx);
	bef_effect_result_t ret = bef_effect_ai_composer_update_node(m_effectHandle, node, key, intensity);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_composer_update_node");
}

void EffectHandle::removeComposerNodes(const std::vector<std::string> &nodes)
{
	std::lock_guard<std::mutex> al(m_renderMtx);
	if (nodes.size() == 0) {
		return;
	}
	char **nodesPath = (char **)malloc(nodes.size() * sizeof(char *));

	std::set<std::string> nodeSet;
	int count = 0;
	for (int i = 0; i < nodes.size(); ++i) {
		const std::string &node = nodes[i];

		if (nodeSet.find(node) != nodeSet.end()) {
			//去除重复的节点
			continue;
		}

		nodeSet.insert(node);

		if (!nodes[i].empty()) {
			nodesPath[count] = (char *)nodes[i].c_str();
		}
		count++;
	}

	bef_effect_result_t ret = BEF_RESULT_SUC;

	ret = bef_effect_ai_composer_remove_nodes(m_effectHandle, (const char **)nodesPath, count);

	if (ret != BEF_RESULT_SUC) {
		printf("bef_effect_ai_composer_remove_nodes error:%d\n", ret);
	}

	if (nodesPath != nullptr) {
		free(nodesPath);
	}
}

void EffectHandle::setFilter(const char *path)
{
	std::lock_guard<std::mutex> al(m_renderMtx);
	bef_effect_result_t ret = BEF_RESULT_SUC;
	ret = bef_effect_ai_set_color_filter_v2(m_effectHandle, path);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_set_color_filter_v2");
}

void EffectHandle::setFilterIntensity(float intensity)
{
	std::lock_guard<std::mutex> al(m_renderMtx);
	bef_effect_result_t ret = BEF_RESULT_SUC;
	ret = bef_effect_ai_set_intensity(m_effectHandle, BEF_INTENSITY_TYPE_GLOBAL_FILTER_V2, intensity);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_set_intensity");
}

void EffectHandle::setSticker(const char *path)
{
	std::lock_guard<std::mutex> al(m_renderMtx);
	bef_effect_result_t ret = BEF_RESULT_SUC;
	ret = bef_effect_ai_set_effect(m_effectHandle, path);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_set_effect");
}

void EffectHandle::setRenderCacheTexture(const char *key, const char *path)
{
	std::lock_guard<std::mutex> al(m_renderMtx);
	bef_effect_result_t ret = BEF_RESULT_SUC;
	ret = bef_effect_ai_set_render_cache_texture(m_effectHandle, key, path);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_set_render_cache_texture");
}

void EffectHandle::setRenderCacheTexture(const char *key, BEFrameInfo *frameInfo)
{
	std::lock_guard<std::mutex> al(m_renderMtx);
	bef_effect_result_t ret = BEF_RESULT_SUC;
	bef_ai_image befImage;
	befImage.data = frameInfo->frame.imageData;
	befImage.format = frameInfo->pixelFormat;
	befImage.width = frameInfo->frame.width;
	befImage.height = frameInfo->frame.height;
	befImage.stride = frameInfo->frame.width * frameInfo->frame.channel;
	befImage.rotate = BEF_AI_CLOCKWISE_ROTATE_0;
	ret = bef_effect_ai_set_render_cache_texture_with_buffer(m_effectHandle, key, &befImage);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_set_render_cache_texture_with_buffer");
}

void EffectHandle::processTouchEvent(BETouchEventType eventType, float x, float y, float force, float majorRadius, int pointerId, int pointerCount)
{
	bef_effect_result_t ret =
		bef_effect_ai_process_touch(m_effectHandle, (bef_ai_touch_event_code)eventType, x, y, force, majorRadius, pointerId, pointerCount);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_process_touch");
}

void EffectHandle::processGestureEvent(BEGestureEventType eventType, float x, float y, float dx, float dy, float factor)
{
	bef_effect_result_t ret = bef_effect_ai_process_gesture(m_effectHandle, (bef_ai_gesture_event_code)eventType, x, y, dx, dy, factor);
	CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, "bef_effect_ai_process_gesture");
}

void EffectHandle::setNeedLoadResource(bool needLoadResource)
{
	RenderLoadResource data;
	commit(m_input, &data);
}

ULONGLONG EffectHandle::getLastRenderTime()
{
	return m_lastRenderTime;
}

GLenum EffectHandle::getReadPixelFormat()
{
	GLenum glformat = GL_RGBA;
	switch (m_format) {
	case BEF::color_i420:
		glformat = GL_RED;
		break;
	default:
		break;
	}
	return glformat;
}

BEF::BEFEffectGLContext *EffectHandle::glContext()
{
	return &m_effectGlContext;
}

bef_effect_handle_t EffectHandle::effectHandle()
{
	return m_effectHandle;
}
}
