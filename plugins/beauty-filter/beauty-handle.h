#pragma once

#include <obs.h>
#include <qmutex.h>
#include <qlist.h>
#include "BEDataManager.h"
#include <QJsonArray>

#define TENCENT_BEAUTY
#include "XMagicHandle.h"

enum class EffectType : int {
	EffectBeauty = 1,
	EffectFilter,
	EffectSticker,
	EffectMakeUp,
};

namespace BYTED_EFFECT {
class IEffectHandle;
}

class BeautyHandle {
public:
	BeautyHandle(obs_source_t *context);
	~BeautyHandle();
	obs_source_frame *processFrame(obs_source_frame *frame);
	void updateBeautySettings(obs_data_t *beautySetting);

private:
	void setDefaultEffect();
	void checkBeautySettings();
	void updateComposerNode(int subId, float value);
	void updateComposerNodeWithKey(int subId, float value, QString key);
	void removeComposerNode(int subId);

	void setFilter(QString filterName);
	void setFilterIntensity(float intensity);

	void setSticker(QString stickerName);

	
	void parseBeauty(const QJsonArray &beautyArr);
	void parseFilter(const QJsonArray &filterArr);
	void parseMakeUp(const QJsonArray &makeUpArr );
	void parseSticker(const QJsonArray& stickerArr);

private:
	obs_source_t *m_source;
	BYTED_EFFECT::IEffectHandle *m_effectHandle;
	std::map<int, BEComposerNodeModel *> m_mapSelNodes;
	BEDataManager m_dataManager;

	bool m_beautyEnabled = false;
	bool m_stickerEnabled = false;
	bool m_filterEnabled = false;
	bool m_makeUpEnable = false;
	QMutex m_beautifySettingMutex;
	QList<QString> m_beautifySettings;

#ifdef TENCENT_BEAUTY
	XMagicHandle *m_pXmagicHandle = nullptr;
#endif
};
