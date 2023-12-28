#pragma once

#include <functional>

void start_rtmp_server(std::function<void(enum class media_status)> state_cb, std::function<void(uint8_t *, size_t, int64_t, bool)> data_cb);
void stop_rtmp_server();
