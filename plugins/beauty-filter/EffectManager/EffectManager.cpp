#include "EffectManager.h"
#include "EffectHandle.h"
#include "bef_effect_ai_load_library.h"


BYTED_EFFECT::IEffectManager* __stdcall createEffectManager() {
	BYTED_EFFECT::EffectManager* manager = new BYTED_EFFECT::EffectManager();
	manager->loadEGLDLL();
	return manager;
}

void __stdcall destroyEffectManager(BYTED_EFFECT::IEffectManager* manager) {
	if (manager) {
		manager->unloadEGLDLL();
		delete manager;
	}
}

namespace BYTED_EFFECT {

	EffectManager::EffectManager() {

	}

	EffectManager::~EffectManager() {

	}

	IEffectHandle* EffectManager::createHandle() {
		return new EffectHandle();
	}

	void EffectManager::releaseHandle(IEffectHandle* handle) {
		if (handle) {
			handle->release();
			delete handle;
			handle = nullptr;
		}
	}

#if USE_DYNAMIC_ANGLE
	static angle::Library* eglLibrary = nullptr;
	static angle::Library* glesLibrary = nullptr;

	static angle::GenericProc KHRONOS_APIENTRY getEGLProcAddress(const char* symbol) {
		return reinterpret_cast<angle::GenericProc>(eglLibrary->getSymbol(symbol));
	}

	static angle::GenericProc KHRONOS_APIENTRY getGLESProcAddress(const char* symbol) {
		return reinterpret_cast<angle::GenericProc>(glesLibrary->getSymbol(symbol));
	}
#endif

	void EffectManager::loadEGLDLL() {
#if USE_DYNAMIC_ANGLE
		std::string eglLibraryPath;
		std::string glesLibraryPath;
#if BEF_EFFECT_PLATFORM == BEF_EFFECT_PLATFORM_WINDOWS
#ifdef DEBUG
		eglLibraryPath = "libEGLd.dll";
		glesLibraryPath = "libGLESv2d.dll";
#else
		eglLibraryPath = "libEGL.dll";
		glesLibraryPath = "libGLESv2.dll";
#endif
#elif BEF_EFFECT_PLATFORM == BEF_EFFECT_PLATFORM_MAC
		eglLibraryPath = "libEGL.dylib";
		glesLibraryPath = "libGLESv2.dylib";
#elif BEF_EFFECT_PLATFORM == ANGLE_PLATFORM_LINUX
		eglLibraryPath = "libEGL.so";
		glesLibraryPath = "libGLESv2.so";
#endif
		eglLibrary = angle::OpenSharedLibrary(eglLibraryPath.c_str(), angle::SearchType::ApplicationDir);
		if (eglLibrary)
		{
			angle::LoadEGL(getEGLProcAddress);
		}

		glesLibrary = angle::OpenSharedLibrary(glesLibraryPath.c_str(), angle::SearchType::ApplicationDir);
		if (glesLibrary)
		{
			angle::LoadGLES(getGLESProcAddress);
		}

		bef_effect_ai_load_egl_library_with_func(getEGLProcAddress);
		bef_effect_ai_load_glesv2_library_with_func(getGLESProcAddress);
#endif
	}

	void EffectManager::unloadEGLDLL() {
#if USE_DYNAMIC_ANGLE
		if (eglLibrary)
		{
			angle::CloseSharedLibrary(eglLibrary);
			eglLibrary = nullptr;
		}

		if (glesLibrary)
		{
			angle::CloseSharedLibrary(glesLibrary);
			glesLibrary = nullptr;
		}
#endif
	}

};
