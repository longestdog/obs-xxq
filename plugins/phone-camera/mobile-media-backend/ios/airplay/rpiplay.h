#pragma once

#include <functional>
#include "../../../common.h"

bool start_airplay_server(std::function<void(media_status)> state_cb, std::function<void(uint8_t *, size_t, int64_t, bool)> data_cb);
void stop_airplay_server();
