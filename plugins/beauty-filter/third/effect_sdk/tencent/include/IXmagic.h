
#ifndef XMAGIC_H_
#define XMAGIC_H_

#ifdef XMAGIC_EXPORTS
#define XMAGIC_API __declspec(dllexport)
#else
#define XMAGIC_API __declspec(dllimport)
#endif
#include <stdint.h>

namespace xmagic {



	/// @brief 美颜类型
	enum class Category {

		BEAUTY = 0,//美颜
		BODY_BEAUTY,//美体
		LUT,//滤镜
		MOTION,//动效
		SEGMENTATION,//扣背
		MAKEUP,//化妆
		ASSET_DATA//模板属性
	};

	class XmagicProperty {
	public:
		XmagicProperty() = default;
		virtual ~XmagicProperty() = default;
		Category category;
		const char* id = nullptr;
		const char* resPath = nullptr;
		const char* effKey = nullptr;
		const char* effValue = nullptr;
		bool isAuth = true;
	};

	/// @brief 像素格式
	enum class PixelFormat {

		PixelFormatRGBA32 = 0,
		PixelFormatBGRA32 = 1
	};

	/// @brief 相机角度
	enum class XmagicCameraOrientation {
		ROTATION_0 = 0,
		ROTATION_90 = 1,
		ROTATION_180 = 2,
		ROTATION_270 = 3,
	};

	/// @brief 图片原点位置
	enum class XmagicImageOrigin {
		TopLeft = 0,
		BottomLeft = 1,
	};

	enum class XmagicYuvFormatType
	{
		YuvFormat601FullRange = 1,
		YuvFormat601VideoRange,
		YuvFormat709FullRange,
		YuvFormat709VideoRange,
	};

	struct YTImagePixelData {
		/// 【字段含义】像素格式
		PixelFormat pixelFormat;

		/// 【字段含义】
		uint8_t* data;

		/// 【字段含义】数据的长度，单位是字节
		int32_t length;

		/// 【字段含义】宽度
		int32_t width;

		/// 【字段含义】高度
		int32_t height;

		YTImagePixelData() : pixelFormat(PixelFormat::PixelFormatRGBA32), data(nullptr), length(0), width(0), height(0) {}
	};

	class IXmagic
	{
	public:
		IXmagic() = default;
		virtual ~IXmagic() = default;

		/// @brief 设置xmagic美颜参数
		/// @param property 对应的设置属性
		virtual void updateProperty(XmagicProperty* property) = 0;

		/// @brief 设置xmagic美颜参数
		/// @param property 对应的设置属性
		/// @param mergeWithCurrentMotion 是否合并动效
		virtual void updateProperty(XmagicProperty* property,bool mergeWithCurrentMotion) = 0;

		///xmagic美颜设置新接口
		virtual void setEffect(const char* effectName, int effectValue, const char* resourcePath, const char* extraInfoJson) = 0;

		/// @brief 处理美颜
		/// @param srcImage 输入美颜数据 
		/// @param dstImage 输出美颜数据
		virtual void process(YTImagePixelData* srcImage, YTImagePixelData* dstImage) = 0;


		/// @brief 处理美颜
		/// @param srcImage 输入美颜数据 
		/// @param dstImage 输出美颜数据
		virtual void process(YTImagePixelData* srcImage, YTImagePixelData* dstImage, XmagicCameraOrientation orientation, XmagicImageOrigin origin) = 0;

		/// @brief 处理纹理接口，返回处理后的纹理（-1说明业务处理异常）
		/// @param textureId 输入美颜纹理 
		virtual int process(int textureId, int width, int height) = 0;

		/// @brief 处理纹理接口，返回处理后的纹理（-1说明业务处理异常）
		/// @param textureId 输入美颜纹理 
		virtual int process(int textureId, int width, int height, XmagicCameraOrientation orientation, XmagicImageOrigin origin) = 0;

		/// @brief i420转化为rgb纹理，返回纹理id
		/// @param yuv_data 数据
		/// @param width 宽
		/// @param height 高
		/// @param yuv标准 601或701
		virtual int I420ToRgbTexture(unsigned char* i420_data,int width,int height, XmagicYuvFormatType format) = 0;

		/// @brief rgb纹理转化为i420数据
		/// @param textureId
		/// @param 输出yuv_data 数据，调用业务申请内存空间
		/// @param width 宽
		/// @param height 高
		/// @param yuv标准 601或701
		virtual void RgbTextureToI420(int textureId,unsigned char* out_i420_data, int width, int height, XmagicYuvFormatType format) = 0;

		///@brief传入时长 timeOffset|处理方式|
		/// :---- | :---- |
		///超过200秒|认为传入值是200秒|
		///少于视频本身时长|截取前timeOffset毫秒循环播放|
		///超过视频本身时长|超出时长播放视频最后一帧，循环播放|

		virtual void setSegmentBg(const char* segmentBgName, int segmentBgType, int timeOffset) = 0;

		/// @brief 暂停
		virtual void onPasue() = 0;

		/// @brief 开始
		virtual void onResume() = 0;

		/// @brief 重置纹理输入大小
		virtual void setRenderSize(int width, int height) = 0;

		///@brief清除美颜效果
		virtual void clearXmagicEffect() = 0;
	};

	class XmagicConstant
	{
	public:
		static constexpr const char* EFFECT_LUT = "lut";//滤镜
		static constexpr const char* EFFECT_MAKEUP = "makeup";//美妆
		static constexpr const char* EFFECT_MOTION = "motion";//动效
		static constexpr const char* EFFECT_SEGMENTATION = "segmentation";//分割

		static constexpr const char* BEAUTY_WHITEN = "beauty.lutFoundationAlpha";
		static constexpr const char* BEAUTY_WHITEN_2 = "beauty.lutFoundationAlpha2";
		static constexpr const char* BEAUTY_WHITEN_3 = "beauty.lutFoundationAlpha3";
		static constexpr const char* BEAUTY_SMOOTH = "smooth.smooth";
		static constexpr const char* BEAUTY_ROSY = "smooth.rosy";
		static constexpr const char* BEAUTY_CONTRAST = "beauty.imageContrastAlpha";
		static constexpr const char* BEAUTY_SATURATION = "smooth.saturation";
		static constexpr const char* BEAUTY_CLEAR = "beauty.lutClearAlpha";
		static constexpr const char* BEAUTY_SHARPEN = "smooth.sharpen";
		static constexpr const char* BEAUTY_ENLARGE_EYE = "basicV7.enlargeEye";
		static constexpr const char* BEAUTY_EYE_LIGHTEN = "beauty.eyeLighten";
		static constexpr const char* BEAUTY_EYE_DISTANCE = "basicV7.eyeDistance";
		static constexpr const char* BEAUTY_EYE_ANGLE = "basicV7.eyeAngle";
		static constexpr const char* BEAUTY_EYE_WIDTH = "basicV7.eyeWidth";
		static constexpr const char* BEAUTY_EYE_HEIGHT = "basicV7.eyeHeight";
		static constexpr const char* BEAUTY_FACE_REMOVE_EYE_BAGS = "beauty.removeEyeBags";
		static constexpr const char* BEAUTY_EYEBROW_ANGLE = "basicV7.eyebrowAngle";
		static constexpr const char* BEAUTY_EYEBROW_DISTANCE = "basicV7.eyebrowDistance";
		static constexpr const char* BEAUTY_EYEBROW_HEIGHT = "basicV7.eyebrowHeight";
		static constexpr const char* BEAUTY_EYEBROW_LENGTH = "basicV7.eyebrowLength";
		static constexpr const char* BEAUTY_EYEBROW_THICKNESS = "basicV7.eyebrowThickness";
		static constexpr const char* BEAUTY_EYEBROW_RIDGE = "basicV7.eyebrowRidge";
		static constexpr const char* BEAUTY_NOSE_THIN = "basicV7.thinNose";
		static constexpr const char* BEAUTY_NOSE_WING = "basicV7.noseWing";
		static constexpr const char* BEAUTY_NOSE_HEIGHT = "basicV7.noseHeight";
		static constexpr const char* BEAUTY_TOOTH_WHITEN = "beauty.toothWhiten";
		static constexpr const char* BEAUTY_MOUTH_SIZE = "basicV7.mouthSize";
		static constexpr const char* BEAUTY_MOUTH_HEIGHT = "basicV7.mouthHeight";
		static constexpr const char* BEAUTY_MOUTH_WIDTH = "basicV7.mouthWidth";
		static constexpr const char* BEAUTY_MOUTH_POSITION = "basicV7.mouthPosition";
		static constexpr const char* BEAUTY_SMILE_FACE = "basicV7.smileFace";
		static constexpr const char* BEAUTY_FACE_THIN = "basicV7.thinFace";
		static constexpr const char* BEAUTY_FACE_NATURE = "basicV7.natureFace";
		static constexpr const char* BEAUTY_FACE_GODNESS = "basicV7.godnessFace";
		static constexpr const char* BEAUTY_FACE_MALE_GOD = "basicV7.maleGodFace";
		static constexpr const char* BEAUTY_FACE_V = "basicV7.vFace";
		static constexpr const char* BEAUTY_FACE_JAW = "basicV7.faceJaw";
		static constexpr const char* BEAUTY_FACE_SHORT = "basicV7.shortFace";
		static constexpr const char* BEAUTY_FACE_BASIC = "liquefaction.basic3";
		static constexpr const char* BEAUTY_FACE_THIN_CHIN = "basicV7.chin";
		static constexpr const char* BEAUTY_FACE_FOREHEAD = "basicV7.forehead";
		static constexpr const char* BEAUTY_FACE_REMOVE_WRINKLE = "beauty.removeWrinkle";
		static constexpr const char* BEAUTY_FACE_REMOVE_LAW_LINE = "beauty.removeLawLine";
		static constexpr const char* BEAUTY_FACE_THIN_CHEEKBONE = "basicV7.cheekboneThin";
		static constexpr const char* BEAUTY_MOUTH_LIPSTICK = "beauty.faceFeatureLipsLut";
		static constexpr const char* BEAUTY_FACE_RED_CHEEK = "beauty.faceFeatureRedCheek";
		static constexpr const char* BEAUTY_FACE_SOFTLIGHT = "beauty.faceFeatureSoftlight";
		static constexpr const char* BEAUTY_FACE_MAKEUP_EYE_SHADOW = "beauty.faceFeatureEyesMakeup.eyeShadow";
		static constexpr const char* BEAUTY_FACE_MAKEUP_EYE_LINER = "beauty.faceFeatureEyesMakeup.eyeLiner";
		static constexpr const char* BEAUTY_FACE_MAKEUP_EYELASH = "beauty.faceFeatureEyesMakeup.eyelash";
		static constexpr const char* BEAUTY_FACE_MAKEUP_EYEBROW = "beauty.faceFeatureEyesMakeup.eyebrow";
		static constexpr const char* BEAUTY_FACE_MAKEUP_EYEBALL = "beauty.faceFeatureEyesMakeup.eyeball";
		static constexpr const char* BODY_AUTOTHIN_BODY_STRENGTH = "body.autothinBodyStrength";
		static constexpr const char* BODY_LEG_STRETCH = "body.legStretch";
		static constexpr const char* BODY_SLIM_LEG_STRENGTH = "body.slimLegStrength";
		static constexpr const char* BODY_WAIST_STRENGTH = "body.waistStrength";
		static constexpr const char* BODY_THIN_SHOULDER_STRENGTH = "body.thinShoulderStrength";
		static constexpr const char* BODY_ENLARGE_CHEST_STRENGTH = "body.enlargeChestStrength";
		static constexpr const char* BODY_SLIM_HEAD_STRENGTH = "body.slimHeadStrength";
	};
}

using namespace xmagic;

#ifdef __cplusplus
extern "C" {
#endif

	/// @brief 设置写日志路径
/// @param path 日志的目录
	XMAGIC_API void setXmagicLogPath(const char* logPath);

	/// @brief xmaigic鉴权，url和key从后台获取
	/// @param url 鉴权url，从后台获取
	/// @param key 鉴权key,从后台获取
	/// @param TELicenseCallback 回调函数，判断鉴权是否成功
	XMAGIC_API void setTELicense(const char* url, const char* key, int (*TELicenseCallback)(int, const char*));


	/// @brief 创建xmaigc对象，对象不使用要销毁
	/// @param resDir 对象的资源目录
	/// @return 返回xmagic
	XMAGIC_API IXmagic* createXmagic(const char* resDir, int width, int height);

	/// @brief 销毁xmagic
	/// @param xmagic 
	XMAGIC_API void destroyXmagic(IXmagic** xmagic);

	/// @brief 初始化qt，非qt界面应用使用。如果qt应用接入，不用初始化
	XMAGIC_API void  needInitQT();

#ifdef __cplusplus
}
#endif
#endif


