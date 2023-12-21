
#ifndef EFFECT_MANAGER_H
#define EFFECT_MANAGER_H


#include "IEffectManager.h"
#include "bef_effect_ai_api.h"

namespace BYTED_EFFECT {
	class EffectManager : public IEffectManager {
	public:
		EffectManager();
		~EffectManager();

	public:
		virtual IEffectHandle* createHandle();
		virtual void releaseHandle(IEffectHandle* handle);
		virtual void loadEGLDLL();
		virtual void unloadEGLDLL();
	};

};

#endif

