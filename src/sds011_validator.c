#include "sds011_validator.h"
#include <string.h>

static bool validate_rep_mode(sds011_msg_t const *req, sds011_msg_t const *res);
static bool validate_dev_id  (sds011_msg_t const *req, sds011_msg_t const *res);
static bool validate_sleep   (sds011_msg_t const *req, sds011_msg_t const *res);
static bool validate_op_mode (sds011_msg_t const *req, sds011_msg_t const *res);

static bool (*_validators[(int)SDS011_MSG_TYPE_COUNT])(sds011_msg_t const *req, sds011_msg_t const *res) = {
  NULL, // reserved
  NULL, // reserved
  validate_rep_mode,
  NULL, // reserved
  NULL, // reserved
  validate_dev_id,
  validate_sleep,
  NULL, // reserved
  validate_op_mode,
};

bool sds011_validator_validate(sds011_msg_t const *req, sds011_msg_t const *res) {
  if (req == NULL || res == NULL) {
    return false;
  }

  if (req->type != res->type) { return false; }
  if (req->op   != res->op  ) { return false; }

  if (req->op == SDS011_MSG_OP_GET) {
    // validation not needed
    return true;
  }

  if (_validators[req->type] != NULL) {
    return _validators[req->type](req, res);
  }

  return true;
}

static bool validate_rep_mode(sds011_msg_t const *req, sds011_msg_t const *res) {
  if (req->data.rep_mode != res->data.rep_mode) {
    return false;
  }
  return true;
}

static bool validate_dev_id(sds011_msg_t const *req, sds011_msg_t const *res) {
  if (req->data.new_dev_id != res->dev_id) {
    return false;
  }
  return true;
}

static bool validate_sleep(sds011_msg_t const *req, sds011_msg_t const *res) {
  if (req->data.sleep != res->data.sleep) {
    return false;
  }
  return true;
}

static bool validate_op_mode(sds011_msg_t const *req, sds011_msg_t const *res) {
  if (req->data.op_mode.interval != res->data.op_mode.interval) {
    return false;
  }
  if (req->data.op_mode.mode != res->data.op_mode.mode) {
    return false;
  }
  return true;
}
