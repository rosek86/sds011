#ifndef SDS011_BUILDER_H__
#define SDS011_BUILDER_H__

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "sds011_common.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t sds011_builder_build(sds011_msg_t const *msg, uint8_t *buf, size_t size);

sds011_err_t sds011_builder_get_error(void);

#ifdef __cplusplus
}
#endif

#endif // SDS011_BUILDER_H__
