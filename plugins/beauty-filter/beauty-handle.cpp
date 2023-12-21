#include "beauty-handle.h"
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include "ModuleHelper.h"
#include "BEResourceProvider.h"
#include "EffectManager/EffectManager.h"

static int logFuncForEffect(int logLevel, const char *msg)
{
	if (msg != nullptr) {
		blog(LOG_DEBUG, "EFFECT_OUTPUT %s", msg);
	}
	return 0;
}

BeautyHandle::BeautyHandle(obs_source_t *context) : m_source(context)
{
	m_effectHandle = ModuleHeper::getInstance().getEffectManager()->createHandle();
	std::string licenseFile = BEResourceProvider::getInstance().licenseDirPath() + "license.bag";
	m_effectHandle->initResource(licenseFile.c_str(), BEResourceProvider::getInstance().modelDirPath().c_str());
	m_effectHandle->setUsePBO(true);
	m_effectHandle->setLogFunc(logFuncForEffect);
	m_dataManager.init(BEEffectType::BEEffectTypeLite);
}

BeautyHandle::~BeautyHandle()
{
	ModuleHeper::getInstance().getEffectManager()->releaseHandle(m_effectHandle);
	m_effectHandle = nullptr;
}

void BeautyHandle::updateComposerNode(int subId, float value)
{
	if (subId == 0) {
		return;
	}

	bool needUpdateNodes = false;
	BEComposerNodeModel *node = nullptr;
	auto iterNode = m_mapSelNodes.find(subId);
	if (iterNode == m_mapSelNodes.end()) {
		node = m_dataManager.getComposerNodeModel(subId);
		if (node != nullptr) {
			m_mapSelNodes.insert(std::make_pair(subId, node));
		}
		needUpdateNodes = true;
	}
	else {
		node = iterNode->second;
	}

	if (needUpdateNodes) {
		std::vector<std::string> nodesPath;
		std::vector<std::string> tags;

		for (auto &selNode : m_mapSelNodes) {
			BEComposerNodeModel *node = selNode.second;
			if (node != nullptr) {
				nodesPath.push_back(node->m_nodePath);
			}
		}

		if (m_effectHandle != nullptr) {
			m_effectHandle->updateComposerNodes(nodesPath, tags);
		}
	}

	if (m_effectHandle != nullptr) {
		std::string key = node->m_vecTag[0].m_key;
		m_effectHandle->updateComposerNodeIntensity(node->m_nodePath.c_str(), key.c_str(), value);
	}
}

void BeautyHandle::updateComposerNodeWithKey(int subId, float value, QString key)
{
	if (subId == 0) {
		return;
	}

	bool needUpdateNodes = false;
	BEComposerNodeModel *node = nullptr;
	auto iterNode = m_mapSelNodes.find(subId);
	if (iterNode == m_mapSelNodes.end()) {
		node = m_dataManager.getComposerNodeModel(subId);
		if (node != nullptr) {
			m_mapSelNodes.insert(std::make_pair(subId, node));
		}
		needUpdateNodes = true;
	}
	else {
		node = iterNode->second;
	}

	if (needUpdateNodes) {
		std::vector<std::string> nodesPath;
		std::vector<std::string> tags;

		for (auto &selNode : m_mapSelNodes) {
			BEComposerNodeModel *node = selNode.second;
			if (node != nullptr) {
				nodesPath.push_back(node->m_nodePath);
			}
		}

		if (m_effectHandle != nullptr) {
			m_effectHandle->updateComposerNodes(nodesPath, tags);
		}
	}

	if (m_effectHandle != nullptr) {
		m_effectHandle->updateComposerNodeIntensity(node->m_nodePath.c_str(), key.toLocal8Bit().data(), value);
	}
}

void BeautyHandle::removeComposerNode(int subId)
{
	auto iterNode = m_mapSelNodes.find(subId);
	if (iterNode != m_mapSelNodes.end()) {
		m_mapSelNodes.erase(iterNode);
	}

	std::vector<std::string> nodesPath;
	std::vector<std::string> tags;

	for (auto &selNode : m_mapSelNodes) {
		BEComposerNodeModel *node = selNode.second;
		if (node != nullptr) {
			nodesPath.push_back(node->m_nodePath);
		}
	}

	if (m_effectHandle != nullptr) {
		m_effectHandle->updateComposerNodes(nodesPath, tags);
	}
}

void BeautyHandle::setFilter(QString filterName)
{
	if (filterName.isEmpty()) {
		m_effectHandle->setFilter("");
		return;
	}
	std::string filterPath = BEResourceProvider::getInstance().filterPath() + filterName.toStdString();
	if (m_effectHandle != nullptr) {
		m_effectHandle->setFilter(filterPath.c_str());
	}
}

void BeautyHandle::setFilterIntensity(float intensity)
{
	if (m_effectHandle != nullptr) {
		m_effectHandle->setFilterIntensity(intensity);
	}
}

void BeautyHandle::setSticker(QString stickerName)
{
	if (m_effectHandle != nullptr) {
		m_effectHandle->setSticker(stickerName.toStdString().c_str());
	}
}

void BeautyHandle::checkBeautySettings()
{
	m_beautifySettingMutex.lock();
	if (!m_beautifySettings.isEmpty()) {
		for (auto iter = m_beautifySettings.begin(); iter != m_beautifySettings.end(); iter++) {
			//设置项总共有美颜 美妆 滤镜 贴纸
			//美颜设置参数=> id value
			//美妆设置参数=> id value
			//贴纸=>传个文件夹的名字
			//滤镜=>传滤镜文件夹的名字，切换滤镜的值需要有一个id，id有一个对应关系
			QJsonDocument jd = QJsonDocument::fromJson((*iter).toUtf8());
			auto p = jd.object();
			int type = p["type"].toInt();
			auto arr = p["beautyParam"].toArray();
			if (type == 1) { //设置美颜
				for (int i = 0; i < arr.size(); i++) {
					auto beautyOne = arr.at(i).toObject();
					int id = beautyOne["id"].toInt();
					float value = beautyOne["value"].toInt() / 100.0;
					bool isAdd = beautyOne["isAdd"].toBool();
					if (isAdd)
						updateComposerNode(id, value);
					else
						removeComposerNode(id);
				}
			}
			else if (type == 2) { //设置滤镜
				for (int i = 0; i < arr.size(); i++) {
					auto beautyOne = arr.at(i).toObject();
					int subType = beautyOne["subType"].toInt();
					if (subType == 0) {
						setFilterIntensity((float)beautyOne["value"].toInt() / 100.f);
					}
					else if (subType == 1) {
						auto filterPath = beautyOne["path"].toString();
						setFilter(filterPath);
						m_filterEnabled = !filterPath.isEmpty();
					}
				}
			}
			else if (type == 3) { //设置贴纸
				for (int i = 0; i < arr.size(); i++) {
					auto beautyOne = arr.at(i).toObject();
					auto stickerPath = beautyOne["path"].toString();
					setSticker(stickerPath);
					m_stickerEnabled = !stickerPath.isEmpty();
				}
			}
			else if (type == 4) { //设置风格妆
				for (int i = 0; i < arr.size(); i++) {
					auto beautyOne = arr.at(i).toObject();
					int id = beautyOne["id"].toInt();
					float value = beautyOne["value"].toInt() / 100.0;
					bool isAdd = beautyOne["isAdd"].toBool();
					bool isFilter = beautyOne["isFilter"].toBool();
					if (isAdd)
						updateComposerNodeWithKey(id, value, isFilter ? "Filter_ALL" : "Makeup_ALL");
					else
						removeComposerNode(id);
				}
			}
		}
		m_beautifySettings.clear();
		m_beautyEnabled = m_mapSelNodes.size() > 0 || m_stickerEnabled || m_filterEnabled;
	}
	m_beautifySettingMutex.unlock();
}

void BeautyHandle::setDefaultEffect()
{
	auto &defaultEffects = m_dataManager.getDefaultEffects();

	std::vector<std::string> nodesPath;
	std::vector<std::string> tags;
	std::vector<float> values;

	for (auto iter : defaultEffects) {
		BEComposerNodeModel *node = m_dataManager.getComposerNodeModel(iter.first);
		if (node != nullptr) {
			m_mapSelNodes.insert(std::make_pair(iter.first, node));
			nodesPath.push_back(node->m_nodePath);
			tags.push_back(node->m_vecTag[0].m_key);
			values.push_back(iter.second);
		}
	}

	m_effectHandle->updateComposerNodes(nodesPath, tags);

	int nodeSize = nodesPath.size();
	for (int i = 0; i < nodeSize; ++i) {
		m_effectHandle->updateComposerNodeIntensity(nodesPath[i].c_str(), tags[i].c_str(), values[i]);
	}
	m_effectHandle->setNeedLoadResource(true);
}

obs_source_frame *BeautyHandle::processFrame(obs_source_frame *frame)
{
	static bool needDrop = false;
	checkBeautySettings();
	bool doBeauty = m_beautyEnabled;
	auto settings = obs_source_get_settings(m_source);
	obs_data_set_int(settings, "need_beauty", doBeauty ? 1 : 0);
	obs_data_release(settings);
	if (!doBeauty || frame->format != VIDEO_FORMAT_RGBA) {
		needDrop = true;
		return frame;
	}
	if (!m_beautyEnabled)
		return frame;

	int width = frame->width;
	int height = frame->height;

	m_effectHandle->setInput(width, height, frame->data[0], BYTED_EFFECT::pixel_format_rgba);

	if (needDrop)
		needDrop = false;
	else {
		unsigned char *outputData = m_effectHandle->getOutput();
		memcpy(frame->data[0], outputData, frame->width * frame->height * 4);
	}

	return frame;
}

void BeautyHandle::updateBeautySettings(obs_data_t *beautySetting)
{
	const char *s = obs_data_get_string(beautySetting, "beautifySetting");
	QMutexLocker locker(&m_beautifySettingMutex);
	m_beautifySettings.append(s);
}
