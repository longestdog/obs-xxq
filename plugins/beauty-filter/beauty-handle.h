#pragma once

#include <obs.h>
#include <qmutex.h>
#include <qlist.h>
#include "BEDataManager.h"

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

private:
	obs_source_t *m_source;
	BYTED_EFFECT::IEffectHandle *m_effectHandle;
	std::map<int, BEComposerNodeModel *> m_mapSelNodes;
	BEDataManager m_dataManager;

	bool m_beautyEnabled = false;
	bool m_stickerEnabled = false;
	bool m_filterEnabled = false;
	QMutex m_beautifySettingMutex;
	QList<QString> m_beautifySettings;
};
