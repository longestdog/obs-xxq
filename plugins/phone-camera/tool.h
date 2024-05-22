#pragma once

#include <obs.h>
#include "common.h"

void dumpResourceImgs();
gs_texture_t *prepareStateTexture(PhoneType type, media_status status);
void closeWindowsFireWall();
int findProcessListeningToPort(int port);
