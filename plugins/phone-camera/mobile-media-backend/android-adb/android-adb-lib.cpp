#include "android-adb-lib.h"
#include "server.h"
#include "device.h"
#include "stream.h"
#include "common.h"
#include <fcntl.h>
#include <io.h>
#include <util/platform.h>

struct server g_s;
struct stream g_stream;
bool g_exit = false;

static bool is_server_running()
{
	return g_s.serial && g_s.video_socket;
}

static void server_clear()
{
	stream_stop(&g_stream);

	server_stop(&g_s);
	server_destroy(&g_s);
	memset(&g_s, 0, sizeof(struct server));
}

static bool server_init(const char *device)
{
	server_init(&g_s);
	struct server_params p;
	memset(&p, 0, sizeof(struct server_params));
	p.bit_rate = 8000000;
	p.port_range.first = 2543;
	p.port_range.last = 2599;
	p.max_fps = 60;
	p.lock_video_orientation = -1;
	p.max_size = 1280;

	if (!server_start(&g_s, device, &p))
		goto err;

	if (!server_connect_to(&g_s))
		goto err;

	char device_name[DEVICE_NAME_FIELD_LENGTH];
	struct size frame_size;
	if (!device_read_info(g_s.video_socket, device_name, &frame_size)) {
		goto err;
	}

	stream_init(&g_stream, g_s.video_socket);
	if (!stream_start(&g_stream))
		goto err;

	return true;
err:
	server_clear();
	return false;
}

std::list<std::string> get_adb_devices()
{
	std::list<std::string> ret;
	int count = 0;
	char **devices;
	adb_devices(&count, &devices);
	for (int i = 0; i < count; i++) {
		ret.emplace_back(devices[i]);
		free(devices[i]);
	}
	free(devices);

	return ret;
}

bool start_android_adb_server(const char *device, std::function<void(media_status)> state_cb, std::function<void(uint8_t *, size_t, int64_t, bool)> data_cb)
{
	g_stream.state_cb = state_cb;
	g_stream.data_cb = data_cb;
	net_init();
	return server_init(device);
}

void stop_android_adb_server()
{
	server_clear();
	net_cleanup();
}
