#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <stdio.h>

#include "adb-config.h"
extern "C" {
#include "util/net.h"
}
#include <functional>
#include "../../common.h"

struct video_buffer;

struct stream {
	socket_t socket;
	pthread_t thread;
	bool stop;
	std::function<void(media_status)> state_cb;
	std::function<void(uint8_t *, size_t, int64_t, bool)> data_cb;
};

void stream_init(struct stream *stream, socket_t socket);

bool stream_start(struct stream *stream);

void stream_stop(struct stream *stream);

void stream_join(struct stream *stream);
