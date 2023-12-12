#ifndef IEFFECT_MANAGER_H
#define IEFFECT_MANAGER_H


#include "EffectManagerDefine.h"
#include <string>
#include <vector>

namespace BYTED_EFFECT {

	typedef enum {
		pixel_format_unknown = 0,
		pixel_format_rgba,  // channel = 4 
		pixel_format_rgb,   // channel = 3 
		pixel_format_bgra,  // channel = 4 
		pixel_format_bgr,   // channel = 3 
		pixel_format_yuy2,
		pixel_format_i420
	} pixel_format_type;

	typedef enum {
		BEGestureEventTap = 0,                 ///< 单击手势
		BEGestureEventPan,                 ///< 滑动手势
		BEGestureEventRotate,              ///< 旋转手势
		BEGestureEventScale,               ///< 缩放手势
		BEGestureEventLongPress,          ///< 长按手势
		BEGestureEventLongDoubleClick         ///< 双击手势
	} BEGestureEventType;

	typedef enum {
		BETouchEventTypeBegin = 0,      ///< 触摸开始事件
		BETouchEventTypeMoved,         ///< 触摸移动事件
		BETouchEventTypeStationary,    ///< 触摸驻停事件
		BETouchEventTypeEnded,         ///< 触摸结束事件
		BETouchEventTypeCanceled     ///< 触摸取消事件
	} BETouchEventType;

	typedef struct {
		unsigned char* imageData;   // 图像buffer 
		int width;      // 图像宽
		int height;     // 图像高
		int channel;    // 图像通道
	} BEVideoFrame;

	typedef struct {
		int oritation;          // 图像方向
		int pixelFormat;        // 图像格式,     RGBA: 0 BGRA 1 BGR 2 RGB 3
		BEVideoFrame frame;     // 图像帧数据
	} BEFrameInfo;



	typedef int(*logEffectFuncPointer)(int logLevel, const char* msg);

	class IRenderCallback {
	public:
		virtual void renderFinish(BEFrameInfo* input, unsigned int outTexture) {}
	};

	class IRenderMsgDelegate {
	public:
		virtual void msgProc(unsigned int uMsgID, int arg1, int arg2, const char* arg3) {}
	};



	class IEffectHandle {
	public:
		virtual ~IEffectHandle() {}

		virtual void release() = 0;
	public:
		virtual void initResource(const char* licensePath, const char* modelDir) = 0;
		// 设置使用PBO加速
		virtual void setUsePBO(bool v) = 0;

		// 设置渲染回调
		virtual void setRenderCallback(IRenderCallback* callback) = 0;

		/*
		 * @brief: 设置log回调，用于排查异常问题
		 */
		virtual void setLogFunc(logEffectFuncPointer logFunc) = 0;

		/*
		 * @brief: 设置消息回调代理
		 */
		virtual void setMsgDelegate(IRenderMsgDelegate* renderMsgDelegate) = 0;

		/*
		 * @brief: 设置输入图像buffer
		 * @param: width 宽
		 * @param: height 高
		 * @param: imageData 图像内存
		 * @param: format 图像格式，目前支持 rgba, rgb, bgra, bgr, i420
		 */
		virtual void setInput(int width, int height, unsigned char* imageData, pixel_format_type format) = 0;

		/*
		 * @brief: 获取渲染后的图像buffer
		 */
		virtual unsigned char* getOutput() = 0;

	public:
		/*
		 * @brief: 更新composer素材
		 */
		virtual void updateComposerNodes(const std::vector<std::string>& nodes, const std::vector<std::string>& tags) = 0;
		virtual void appendComposerNodes(const std::vector<std::string>& nodes, const std::vector<std::string>& tags) = 0;

		/*
		 * @brief: 更新composer素材的强度
		 * @param: node 对应素材的节点
		 * @param: key 素材对应的key
		 * @param: intensity 效果强度 0.0 - 1.0
		 */
		virtual void updateComposerNodeIntensity(const char* node, const char* key, float intensity) = 0;
		virtual void removeComposerNodes(const std::vector<std::string>& nodes) = 0;

		/*
		 * @brief: 设置滤镜
		 * @param: path 滤镜路径(绝对路径)
		 */
		virtual void setFilter(const char* path) = 0;

		/*
		 * @brief: 设置滤镜的强度
		 * @param: intensity 强度值 0.0 - 1.0
		 */
		virtual void setFilterIntensity(float intensity) = 0;

		/*
		 * @brief: 设置贴纸素材
		 * @param: path 贴纸路径(绝对路径)
		 */
		virtual void setSticker(const char* path) = 0;

		/*
		 * @brief: 设置自定义渲染缓存（目前用于虚拟背景）
		 */
		virtual void setRenderCacheTexture(const char* key, const char* path) = 0;
		virtual void setRenderCacheTexture(const char* key, BEFrameInfo* frameInfo) = 0;

		/*
		 * @brief 处理触摸事件
		 * @param eventCode 触摸事件类型
		 * @param x 触摸位置
		 * @param y 触摸位置
		 * @param force 压力值
		 * @param majorRadius 触摸范围
		 * @param pointerId 触摸点 id
		 * @param pointerCount 触摸点数量
		 */
		virtual void processTouchEvent(BETouchEventType eventType, float x, float y, float force, float majorRadius, int pointerId, int pointerCount) = 0;

		/*
		 * @brief 处理鼠标事件
		 * @param eventCode 手势事件类型
		 * @param x 点击位置
		 * @param y 点击位置
		 * @param dx 移动距离
		 * @param dy 移动距离
		 * @param factor 缩放因数
		 */
		virtual void processGestureEvent(BEGestureEventType eventType, float x, float y, float dx, float dy, float factor) = 0;
		/*
		 * @brief: 设置是否同步加载素材（用于初始开启特效时设置默认效果）
		 * @param: needLoadResource true (同步等待) false (默认：异步)
		 */
		virtual void setNeedLoadResource(bool needLoadResource) = 0;

		virtual ULONGLONG getLastRenderTime() = 0;
	};

	class IEffectManager {
	public:
		IEffectManager() {}
		virtual ~IEffectManager() {}
	public:
		/*
		* @brief: 创建内部渲染的effecthandle
		*/
		virtual IEffectHandle* createHandle() = 0;

		/*
		* @brief: 释放内部渲染的effecthandle
		*/
		virtual void releaseHandle(IEffectHandle* handle) = 0;

		/*
		* @brief: 加载libegl库
		*/
		virtual void loadEGLDLL() = 0;

		/*
		* @brief: 释放libegl库
		*/
		virtual void unloadEGLDLL() = 0;
	};



};



BYTED_EFFECT::IEffectManager* __stdcall createEffectManager();
void __stdcall destroyEffectManager(BYTED_EFFECT::IEffectManager* manager);

#endif
