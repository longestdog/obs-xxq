#pragma once

#include <functional>
#include <list>
#include <string>
#include "../../common.h"

std::list<std::string> get_adb_devices();
bool start_android_adb_server(std::function<void(media_status)> state_cb, std::function<void(uint8_t *, size_t, int64_t, bool)> data_cb);
void stop_android_adb_server();
