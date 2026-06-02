#include <util/dstr.h>
#include "dc-capture.h"

/* clang-format off */

#define TEXT_MONITOR_CAPTURE obs_module_text("MonitorCapture_GDI")
#define TEXT_CAPTURE_CURSOR  obs_module_text("CaptureCursor")
#define TEXT_COMPATIBILITY   obs_module_text("Compatibility")
#define TEXT_MONITOR         obs_module_text("Monitor")
#define TEXT_PRIMARY_MONITOR obs_module_text("PrimaryMonitor")

/* clang-format on */

struct monitor_capture {
	obs_source_t *source;

	int monitor;
	bool capture_cursor;
	bool compatibility;

	struct dc_capture data;
};

struct monitor_info {
	int cur_id;
	int desired_id;
	int id;
	RECT rect;
	bool found;
};

#define GDI_CAPTURE_MAX_SIZE 32767

/* ------------------------------------------------------------------------- */

static inline void do_log(int level, const char *msg, ...)
{
	va_list args;
	struct dstr str = {0};

	va_start(args, msg);

	dstr_copy(&str, "[GDI monitor capture]: ");
	dstr_vcatf(&str, msg, args);
	blog(level, "%s", str.array);
	dstr_free(&str);

	va_end(args);
}

static BOOL CALLBACK enum_monitor(HMONITOR handle, HDC hdc, LPRECT rect,
				  LPARAM param)
{
	struct monitor_info *monitor = (struct monitor_info *)param;

	if (monitor->desired_id == monitor->cur_id) {
		MONITORINFO mi = {0};
		mi.cbSize = sizeof(mi);
		if (GetMonitorInfo(handle, &mi))
			monitor->rect = mi.rcMonitor;
		else
			monitor->rect = *rect;
		monitor->id = monitor->cur_id;
		monitor->found = true;
	}

	UNUSED_PARAMETER(hdc);
	UNUSED_PARAMETER(handle);
	return (monitor->desired_id > monitor->cur_id++);
}

static bool get_monitor_dimensions(obs_data_t *settings, int *monitor_id,
				   RECT *rect, uint32_t *width,
				   uint32_t *height)
{
	struct monitor_info monitor = {0};
	LONG monitor_width;
	LONG monitor_height;

	monitor.desired_id = (int)obs_data_get_int(settings, "monitor");
	EnumDisplayMonitors(NULL, NULL, enum_monitor, (LPARAM)&monitor);

	if (!monitor.found) {
		do_log(LOG_WARNING,
		       "Monitor %d not found, skipping capture init",
		       monitor.desired_id);
		return false;
	}

	monitor_width = monitor.rect.right - monitor.rect.left;
	monitor_height = monitor.rect.bottom - monitor.rect.top;

	if (monitor_width <= 0 || monitor_height <= 0 ||
	    monitor_width >= GDI_CAPTURE_MAX_SIZE ||
	    monitor_height >= GDI_CAPTURE_MAX_SIZE) {
		do_log(LOG_WARNING,
		       "Invalid monitor dimensions %ldx%ld (rect: %ld,%ld %ld,%ld)",
		       monitor_width, monitor_height, monitor.rect.left,
		       monitor.rect.top, monitor.rect.right,
		       monitor.rect.bottom);
		return false;
	}

	*monitor_id = monitor.id;
	*rect = monitor.rect;
	*width = (uint32_t)monitor_width;
	*height = (uint32_t)monitor_height;
	return true;
}

static inline void update_settings(struct monitor_capture *capture,
				   obs_data_t *settings)
{
	RECT rect;
	uint32_t width;
	uint32_t height;
	int monitor_id;

	capture->capture_cursor = obs_data_get_bool(settings, "capture_cursor");
	capture->compatibility = obs_data_get_bool(settings, "compatibility");

	if (!get_monitor_dimensions(settings, &monitor_id, &rect, &width,
				    &height)) {
		/* 全屏切换等场景下可能短暂读到非法 rect，保留现有采集 */
		return;
	}

	capture->monitor = monitor_id;

	dc_capture_free(&capture->data);
	dc_capture_init(&capture->data, rect.left, rect.top, width, height,
			capture->capture_cursor, capture->compatibility);
}

/* ------------------------------------------------------------------------- */

static const char *monitor_capture_getname(void *unused)
{
	UNUSED_PARAMETER(unused);
	return TEXT_MONITOR_CAPTURE;
}

static void monitor_capture_destroy(void *data)
{
	struct monitor_capture *capture = data;

	obs_enter_graphics();
	dc_capture_free(&capture->data);
	obs_leave_graphics();

	bfree(capture);
}

static void monitor_capture_defaults(obs_data_t *settings)
{
	obs_data_set_default_int(settings, "monitor", 0);
	obs_data_set_default_bool(settings, "capture_cursor", true);
	obs_data_set_default_bool(settings, "compatibility", false);
}

static void monitor_capture_update(void *data, obs_data_t *settings)
{
	struct monitor_capture *mc = data;
	update_settings(mc, settings);
}

static void *monitor_capture_create(obs_data_t *settings, obs_source_t *source)
{
	struct monitor_capture *capture;

	capture = bzalloc(sizeof(struct monitor_capture));
	capture->source = source;

	update_settings(capture, settings);

	if (!capture->data.valid)
		do_log(LOG_WARNING, "Failed to initialize monitor capture");

	return capture;
}

static void monitor_capture_tick(void *data, float seconds)
{
	struct monitor_capture *capture = data;

	if (!obs_source_showing(capture->source))
		return;

	obs_enter_graphics();
	dc_capture_capture(&capture->data, NULL);
	obs_leave_graphics();

	UNUSED_PARAMETER(seconds);
}

static void monitor_capture_render(void *data, gs_effect_t *effect)
{
	struct monitor_capture *capture = data;
	dc_capture_render(&capture->data,
			  obs_get_base_effect(OBS_EFFECT_OPAQUE));

	UNUSED_PARAMETER(effect);
}

static uint32_t monitor_capture_width(void *data)
{
	struct monitor_capture *capture = data;
	return capture->data.width;
}

static uint32_t monitor_capture_height(void *data)
{
	struct monitor_capture *capture = data;
	return capture->data.height;
}

static BOOL CALLBACK enum_monitor_props(HMONITOR handle, HDC hdc, LPRECT rect,
					LPARAM param)
{
	UNUSED_PARAMETER(hdc);
	UNUSED_PARAMETER(rect);

	obs_property_t *monitor_list = (obs_property_t *)param;
	MONITORINFO mi;
	size_t monitor_id = 0;
	struct dstr monitor_desc = {0};
	struct dstr resolution = {0};
	struct dstr format_string = {0};

	monitor_id = obs_property_list_item_count(monitor_list);

	mi.cbSize = sizeof(mi);
	GetMonitorInfo(handle, &mi);

	dstr_catf(&resolution, "%dx%d @ %d,%d",
		  mi.rcMonitor.right - mi.rcMonitor.left,
		  mi.rcMonitor.bottom - mi.rcMonitor.top, mi.rcMonitor.left,
		  mi.rcMonitor.top);

	dstr_copy(&format_string, "%s %d: %s");
	if (mi.dwFlags == MONITORINFOF_PRIMARY) {
		dstr_catf(&format_string, " (%s)", TEXT_PRIMARY_MONITOR);
	}

	dstr_catf(&monitor_desc, format_string.array, TEXT_MONITOR,
		  monitor_id + 1, resolution.array);

	obs_property_list_add_int(monitor_list, monitor_desc.array,
				  (int)monitor_id);

	dstr_free(&monitor_desc);
	dstr_free(&resolution);
	dstr_free(&format_string);

	return TRUE;
}

static obs_properties_t *monitor_capture_properties(void *unused)
{
	UNUSED_PARAMETER(unused);

	obs_properties_t *props = obs_properties_create();

	obs_property_t *monitors = obs_properties_add_list(
		props, "monitor", TEXT_MONITOR, OBS_COMBO_TYPE_LIST,
		OBS_COMBO_FORMAT_INT);

	obs_properties_add_bool(props, "compatibility", TEXT_COMPATIBILITY);
	obs_properties_add_bool(props, "capture_cursor", TEXT_CAPTURE_CURSOR);

	EnumDisplayMonitors(NULL, NULL, enum_monitor_props, (LPARAM)monitors);

	return props;
}

struct obs_source_info monitor_capture_info = {
	.id = "monitor_capture",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW |
			OBS_SOURCE_DO_NOT_DUPLICATE,
	.get_name = monitor_capture_getname,
	.create = monitor_capture_create,
	.destroy = monitor_capture_destroy,
	.video_render = monitor_capture_render,
	.video_tick = monitor_capture_tick,
	.update = monitor_capture_update,
	.get_width = monitor_capture_width,
	.get_height = monitor_capture_height,
	.get_defaults = monitor_capture_defaults,
	.get_properties = monitor_capture_properties,
};
