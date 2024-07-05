#include "beauty-handle.h"
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <QOpenGLContext>
#include <QSurface>

BeautyHandle::BeautyHandle(obs_source_t *context) : m_source(context)
{
	m_pXmagicHandle = new XMagicHandle(1280, 720);
}

BeautyHandle::~BeautyHandle()
{
	delete m_pXmagicHandle;
	m_pXmagicHandle = nullptr;
}

obs_source_frame *BeautyHandle::processFrame(obs_source_frame *frame)
{
	static bool needDrop = false;
	if (!m_pXmagicHandle) {
		m_pXmagicHandle = new XMagicHandle(frame->width, frame->height);
	}
	bool doBeauty = m_beautyEnabled || m_filterEnabled || m_makeUpEnable || m_stickerEnabled;

	auto settings = obs_source_get_settings(m_source);
	obs_data_set_int(settings, "need_beauty", doBeauty ? 1 : 0);
	obs_data_release(settings);

	if (!doBeauty || frame->format != VIDEO_FORMAT_RGBA) {
		needDrop = true;
		return frame;
	}
	if (!doBeauty)
		return frame;

	int width = frame->width;
	int height = frame->height;

	if (m_pXmagicHandle && frame->data[0] && width > 0 && height > 0) {
		m_pXmagicHandle->setInput(frame->data[0], width, height);
		if (needDrop)
			needDrop = false;
		else {
			unsigned char *outputData = m_pXmagicHandle->getOutput();
			if (outputData)
				std::copy(outputData, outputData + width * height * 4, frame->data[0]);
			//memcpy(frame->data[0], outputData, frame->width * frame->height * 4);
		}
	} else {
		blog(LOG_INFO, "processFrame xMagicHandle: %p, dataInfo: %p,%d,%d", m_pXmagicHandle, frame->data[0],width,height);
	}

	return frame;
}
void BeautyHandle::parseBeauty(const QJsonArray &beautyArr)
{
	for (int i = 0; i < beautyArr.size(); i++) {

		auto beautyOne = beautyArr.at(i).toObject();
		int id = beautyOne["id"].toInt();
		int value = beautyOne["value"].toInt();
		std::string effectKey = beautyOne["effectKey"].toString().toStdString();
		bool isAdd = beautyOne["isAdd"].toBool();
		if (!isAdd) {
			value = 0;
			m_beautyEnabled = false;
		} else {
			m_beautyEnabled = true;
		}
		if (m_pXmagicHandle && !effectKey.empty()) {
			m_pXmagicHandle->addBeauty(effectKey, value);
		}
	}
}

void BeautyHandle::parseFilter(const QJsonArray &filterArr)
{
	for (int i = 0; i < filterArr.size(); i++) {
		auto beautyOne = filterArr.at(i).toObject();
		int subType = beautyOne["subType"].toInt();
		auto path = beautyOne["path"].toString();
		m_filterEnabled = !path.isEmpty();
		if (subType == 0) { //value
			std::string filterPath = path.isEmpty() ? "" : "lut.bundle/" + path.toStdString();
			auto value = beautyOne["value"].toInt();
			if (m_pXmagicHandle) {
				m_pXmagicHandle->addFilter(filterPath, value);
			}
		} else if (subType == 1) {                       //path
			if (path.isEmpty() && m_pXmagicHandle) { //makeup change trigger
				m_pXmagicHandle->addFilter(path.toStdString(), 0);
			}
		}
	}
}

void BeautyHandle::parseMakeUp(const QJsonArray &makeUpArr)
{
	for (int i = 0; i < makeUpArr.size(); i++) {
		auto beautyOne = makeUpArr.at(i).toObject();
		int id = beautyOne["id"].toInt();
		int value = beautyOne["value"].toInt();
		bool isAdd = beautyOne["isAdd"].toBool();
		bool isFilter = beautyOne["isFilter"].toBool();
		QString path = QString("makeupMotionRes.bundle/video_makeup_%1/").arg(beautyOne["path"].toString());
		if (!isAdd)
			path = "";
		if (m_pXmagicHandle) {
			m_makeUpEnable = !path.isEmpty();
			m_pXmagicHandle->addMakeUp(path.toStdString(), value, isFilter);
		}
	}
}

void BeautyHandle::parseSticker(const QJsonArray &stickerArr)
{
	for (int i = 0; i < stickerArr.size(); i++) {
		auto beautyOne = stickerArr.at(i).toObject();
		auto stickerPath = beautyOne["path"].toString();
		if (!stickerPath.isEmpty()) {
			//stickerPath = "2dMotionRes.bundle/video_tutujiang/template.json";
			//stickerPath = "C:/Users/zhouleigang/AppData/Local/yuerlive/cache/stickers/3dMotionRes.bundle/video_zhixingmeigui/template.json";
			stickerPath.append("/template.json");
		}
		m_stickerEnabled = !stickerPath.isEmpty();
		if (m_pXmagicHandle) {
			m_pXmagicHandle->addStrick(stickerPath.toStdString(), 0);
		}
	}
}

void BeautyHandle::updateBeautySettings(obs_data_t *beautySetting)
{
	const char *s = obs_data_get_string(beautySetting, "beautifySetting");
	QJsonDocument jd = QJsonDocument::fromJson((QString(s)).toUtf8());
	auto p = jd.object();
	EffectType type = (EffectType)p["type"].toInt();
	auto arr = p["beautyParam"].toArray();
	switch (type) {
	case EffectType::EffectBeauty:
		parseBeauty(arr);
		break;
	case EffectType::EffectFilter: {
		parseFilter(arr);
	} break;
	case EffectType::EffectSticker: {
		parseSticker(arr);
	} break;
	case EffectType::EffectMakeUp: {
		parseMakeUp(arr);
	} break;
	default:
		break;
	}
}
