#ifndef SDS011_VALIDATOR_H__
#define SDS011_VALIDATOR_H__

#include "sds011_common.h"
#include <stdbool.h>

bool sds011_validator_validate(sds011_msg_t const *req, sds011_msg_t const *res);

#endif // SDS011_VALIDATOR_H__
