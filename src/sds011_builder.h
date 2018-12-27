#ifndef SDS011_BUILDER_H__
#define SDS011_BUILDER_H__

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "sds011_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Serialize message
 * @parma msg Message to be serialized
 * @param buf buffer for the serialized message
 * @param size size of the buffer
 * @return output size, 0 in case of an error
 */
size_t sds011_builder_build(sds011_msg_t const *msg, uint8_t *buf, size_t size);

/**
 * Get latest builder error code
 * @return latest error code
 */
sds011_err_t sds011_builder_get_error(void);

#ifdef __cplusplus
}
#endif

#endif // SDS011_BUILDER_H__
