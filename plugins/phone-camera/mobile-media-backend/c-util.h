#pragma once

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#include <stdint.h>

EXTERNC void usb_device_reset(const char *serial);
EXTERNC void update_driver_process(bool in);
EXTERNC bool in_driver_process();
EXTERNC void add_process(const char *id, void *process);
EXTERNC void remove_process(const char *id);
EXTERNC void kill_driver_process(const char *id);

typedef void (*media_data_cb)(char *buf, int size, void *cb_data);
typedef void (*device_lost)(void *cb_data);
EXTERNC void add_exceptions(const char *serial);
EXTERNC void add_media_callback(const char *serial, media_data_cb cb, device_lost lost_cb, void *cb_data);
EXTERNC void remove_media_callback(const char *serial);
EXTERNC void process_media_data(const char *serial, char *buf, int size);
EXTERNC void process_device_lost(const char *serial);
EXTERNC bool media_callback_installed(const char *serial);
