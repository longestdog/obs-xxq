#ifndef MODULEHELPER_H
#define MODULEHELPER_H

#include "EffectManager/IEffectManager.h"
#include "Algorithm/IAlgorithmManager.h"

class ModuleHeper {
public:
	static ModuleHeper &getInstance();

public:
	BYTED_EFFECT::IEffectManager *getEffectManager();
	BYTED_EFFECT::IAlgorithmManager *getAlgorithmManager();

	void destroyEffectManager();
	void destroyAlgorithmManager();

private:
	ModuleHeper() = default;
	~ModuleHeper();

private:
	BYTED_EFFECT::IEffectManager *m_effectManager;
	BYTED_EFFECT::IAlgorithmManager *m_algorithmManager;
};

#endif // MODULEHELPER_H
