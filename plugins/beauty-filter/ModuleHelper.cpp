#include "ModuleHelper.h"

ModuleHeper& ModuleHeper::getInstance()
{
    static ModuleHeper s_moduleHelper;
    return s_moduleHelper;
}

BYTED_EFFECT::IEffectManager* ModuleHeper::getEffectManager() {
    if (m_effectManager == nullptr) {
        m_effectManager = ::createEffectManager();
    }
    return m_effectManager;
}

BYTED_EFFECT::IAlgorithmManager* ModuleHeper::getAlgorithmManager() {
    if (m_algorithmManager == nullptr) {
        m_algorithmManager = ::createAlgorithmManager(false);
    }
    return m_algorithmManager;
}

void ModuleHeper::destroyEffectManager() {
    if (m_effectManager != nullptr) {
        ::destroyEffectManager(m_effectManager);
        m_effectManager = nullptr;
    }
}

void ModuleHeper::destroyAlgorithmManager() {
    if (m_algorithmManager != nullptr) {
        ::destroyAlgorithmManager(m_algorithmManager);
        m_algorithmManager = nullptr;
    }
}

ModuleHeper::~ModuleHeper() {
    destroyEffectManager();
    destroyAlgorithmManager();
}
