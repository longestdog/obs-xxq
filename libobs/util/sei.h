#pragma once

#include <stdint.h>
#include "util/c99defs.h"

#ifdef __cplusplus
extern "C" {
#endif

EXPORT size_t avc_sei(unsigned char *out, const unsigned char *in, int in_len);
EXPORT size_t hevc_sei(unsigned char *out, const unsigned char *in, int in_len);

#ifdef __cplusplus
}
#endif
