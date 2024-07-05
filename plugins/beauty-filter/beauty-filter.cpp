#include <obs-module.h>
#include <qapplication.h>
#include "beauty-handle.h"

static const char *beauty_filter_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("YPPBeautyFilter");
}

static void beauty_filter_update(void *data, obs_data_t *settings)
{
	UNUSED_PARAMETER(data);
	UNUSED_PARAMETER(settings);
}

static void *beauty_filter_create(obs_data_t *settings, obs_source_t *context)
{
	UNUSED_PARAMETER(settings);
	BeautyHandle *filter = new BeautyHandle(context);
	return filter;
}

static void beauty_filter_destroy(void *data)
{
	BeautyHandle *filter = (BeautyHandle *)data;
	delete filter;
}

static obs_properties_t *beauty_filter_properties(void *data)
{
	obs_properties_t *props = obs_properties_create();

	UNUSED_PARAMETER(data);
	return props;
}

static void beauty_filter_remove(void *data, obs_source_t *parent)
{
	//BeautyHandle *filter = (BeautyHandle *)data;
}

static struct obs_source_frame *beauty_filter_video(void *data, struct obs_source_frame *frame)
{
	BeautyHandle *filter = (BeautyHandle *)data;
	return filter->processFrame(frame);
}

void beauty_custom_command(void *data, obs_data_t *command)
{
	BeautyHandle *filter = (BeautyHandle *)data;
	filter->updateBeautySettings(command);
}

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("beauty-filter", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
	return "ypp beauty filters";
}

#ifdef USE_ANGLE
#ifdef USE_DYNAMIC_ANGLE
#include "egl_loader_autogen.h"
#include "gles_loader_autogen.h"
#include "system_utils.h"
#endif
#endif

#if USE_DYNAMIC_ANGLE
static angle::Library *eglLibrary = nullptr;
static angle::Library *glesLibrary = nullptr;
#endif

// callbacks for loading EGL function pointers.
static angle::GenericProc KHRONOS_APIENTRY getEGLProcAddress(const char *symbol)
{
	return reinterpret_cast<angle::GenericProc>(eglLibrary->getSymbol(symbol));
}
// callbacks for loading GLES function pointers.
static angle::GenericProc KHRONOS_APIENTRY getGLESProcAddress(const char *symbol)
{
	return reinterpret_cast<angle::GenericProc>(glesLibrary->getSymbol(symbol));
}

void loadGraphicLibrary(const std::string &currentAppPath)
{
#if USE_DYNAMIC_ANGLE
	std::string eglLibraryPath;
	std::string glesLibraryPath;
#ifdef DEBUG
	eglLibraryPath = currentAppPath + "libEGLd.dll";
	glesLibraryPath = currentAppPath + "libGLESv2d.dll";
#else
	eglLibraryPath = currentAppPath + "libEGL.dll";
	glesLibraryPath = currentAppPath + "libGLESv2.dll";
#endif // DEBUG

	// angle::OpenSharedLibrary 只负责打开共享库,不会对其中符号进行解析或加载
	eglLibrary = angle::OpenSharedLibrary(eglLibraryPath.c_str(), angle::SearchType::ApplicationDir);
	if (eglLibrary)
		angle::LoadEGL(getEGLProcAddress);
	glesLibrary = angle::OpenSharedLibrary(glesLibraryPath.c_str(), angle::SearchType::ApplicationDir);
	if (glesLibrary)
		angle::LoadGLES(getGLESProcAddress);
#endif
}

void releaseGraphicLibrary()
{
#if USE_DYNAMIC_ANGLE
	if (eglLibrary) {
		angle::CloseSharedLibrary(eglLibrary);
		eglLibrary = nullptr;
	}

	if (glesLibrary) {
		angle::CloseSharedLibrary(glesLibrary);
		glesLibrary = nullptr;
	}
#endif
}

#include "BEResourceProvider.h"
#include "LicenseRequester.h"
bool obs_module_load(void)
{
	struct obs_source_info beauty_filter = { 0 };
	beauty_filter.id = "beauty_filter";
	beauty_filter.type = OBS_SOURCE_TYPE_FILTER;
	beauty_filter.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_ASYNC;
	beauty_filter.get_name = beauty_filter_name;
	beauty_filter.create = beauty_filter_create;
	beauty_filter.destroy = beauty_filter_destroy;
	beauty_filter.update = beauty_filter_update;
	beauty_filter.get_properties = beauty_filter_properties;
	beauty_filter.filter_video = beauty_filter_video;
	beauty_filter.filter_remove = beauty_filter_remove;
	beauty_filter.make_command = beauty_custom_command;

	obs_register_source(&beauty_filter);

#ifndef TENCENT_BEAUTY
	QString appPathQstr = QApplication::applicationDirPath();
	std::string appPath = "";
	appPath = appPathQstr.toUtf8().toStdString();
	if (!appPath.rfind("/") != appPath.length() - 1) {
		appPath.append("/");
	}

	BEResourceProvider::getInstance().initialize(appPath);
	check_update_license();
	loadGraphicLibrary(appPath);
#endif //TENCENT_BEAUTY
	return true;
}

void obs_module_unload()
{
	releaseGraphicLibrary();
}
