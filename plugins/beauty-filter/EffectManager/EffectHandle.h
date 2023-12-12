#ifndef EFFECT_HANDLE_H
#define EFFECT_HANDLE_H

#if USE_DYNAMIC_ANGLE
#include "bef_effect_ai_load_library.h"
#include "egl_loader_autogen.h"
#include "gles_loader_autogen.h"
#include "system_utils.h"
#endif
#include "utils/BEFEffectThread.h"
#include "utils/BEFEffectGLContext.h"
#include "utils/BEFColorConverter.h"
#include "bef_effect_ai_api.h"
#include "bef_effect_ai_message_define.h"
#include "IEffectManager.h"
#include "PBOReader.h"
#include "utils/BEImageUtil.h"
#include <vector>
#include <mutex>

#define MULTI_HANDLE_USE 0
#define EFFECT_MESSAGE_USE 0

namespace BYTED_EFFECT {

	class EffectHandle : public BEF::BEFEffectThread, public IEffectHandle {
	public:
		EffectHandle();
		virtual ~EffectHandle();

		virtual void release() override;
	private:
		// effect渲染回调线程，非主动调用
		virtual void init() override;
		virtual void render(void* userData = nullptr) override;
		virtual void destroy() override;


	public:
		virtual void initResource(const char* licensePath, const char* modelDir) override;
		virtual void setUsePBO(bool v) override;
		virtual void setRenderCallback(IRenderCallback* callback) override;
		virtual void setLogFunc(logEffectFuncPointer logFunc) override;
		virtual void setMsgDelegate(IRenderMsgDelegate* renderMsgDelegate) override;
		virtual void setInput(int width, int height, unsigned char* imageData, pixel_format_type format) override;
		virtual unsigned char* getOutput() override;

		virtual void updateComposerNodes(const std::vector<std::string>& nodes, const std::vector<std::string>& tags) override;
		virtual void appendComposerNodes(const std::vector<std::string>& nodes, const std::vector<std::string>& tags) override;
		virtual void updateComposerNodeIntensity(const char* node, const char* key, float intensity) override;
		virtual void removeComposerNodes(const std::vector<std::string>& nodes) override;
		virtual void setFilter(const char* path) override;
		virtual void setFilterIntensity(float intensity) override;
		virtual void setSticker(const char* path) override;
		virtual void setRenderCacheTexture(const char* key, const char* path) override;
		virtual void setRenderCacheTexture(const char* key, BEFrameInfo* frameInfo) override;

		virtual void processTouchEvent(BETouchEventType eventType, float x, float y, float force, float majorRadius, int pointerId, int pointerCount) override;
		virtual void processGestureEvent(BEGestureEventType eventType, float x, float y, float dx, float dy, float factor) override;

		virtual void setNeedLoadResource(bool needLoadResource) override;

		virtual ULONGLONG getLastRenderTime() override;

		void msgProc(unsigned int uMsgId, int arg1, int arg2, const char* arg3);

		BEF::BEFEffectGLContext *glContext();
		bef_effect_handle_t effectHandle();

	private:
		void pboReadPixel();

		GLenum getReadPixelFormat();

		void createContextWnd();

	private:
		BEF::BEFEffectGLContext m_effectGlContext;
		HWND m_hContextWnd = NULL;

		BEImageUtil* m_imageUtil = nullptr;

		GLuint m_inputTexture = 0;
		GLuint m_outputTexture = 0;

		unsigned char* m_readBuffer = nullptr;


		BEF::bef_color_type m_format = BEF::color_rgba;

		int m_width = 0;
		int m_height = 0;

		ULONGLONG m_lastRenderTime = 0;

		IRenderCallback* m_renderCallback = { nullptr };  // 渲染回调，处理一些外部的渲染实现，
#if EFFECT_MESSAGE_USE
		IRenderMsgDelegate* m_renderMsgDelegate = { nullptr }; // 消息回调接口
		bef_render_msg_delegate_manager m_renderMsgManager;
#endif


		std::string m_licensePath;		// 授权文件路径
		std::string m_modelDir;			// 模型路径

		std::mutex m_renderMtx;

		bool m_bInit = false;
		bool m_isFirstFrame = true;

		bool m_usePBO = false;  // 是否使用pbo的方式读取纹理
		PBOReader* m_pboReader = nullptr;

		bef_effect_handle_t m_effectHandle = 0;
	};

}

#endif
