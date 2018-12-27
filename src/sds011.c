
#include <stdint.h>
#include "sds011.h"

#include "sds011_parser.h"
#include "sds011_builder.h"

// TODO: sds011_t structure
//       read/write
//       timeouts

static sds011_parser_t _parser;
static sds011_init_t _init;
static sds011_cb_t _cb;
static sds011_msg_type_t _cb_msg_type;

sds011_err_t send_msg(sds011_msg_t const *msg, sds011_cb_t cb);

sds011_err_t sds011_init(sds011_init_t const *init) {
  sds011_parser_init(&_parser);
  _init = *init;
  _cb = NULL;
  _cb_msg_type = 0;
  return SDS011_OK;
}

sds011_err_t sds011_query_data(uint16_t dev_id, sds011_cb_t cb) {
  return send_msg(&(sds011_msg_t) {
    .dev_id = dev_id,
    .type   = SDS011_MSG_TYPE_DATA,
    .op     = SDS011_MSG_OP_GET,
    .src    = SDS011_MSG_SRC_HOST
  }, cb);
}

sds011_err_t sds011_set_device_id(uint16_t dev_id, uint16_t new_id, sds011_cb_t cb) {
  return send_msg(&(sds011_msg_t) {
    .dev_id             = dev_id,
    .type               = SDS011_MSG_TYPE_DEV_ID,
    .op                 = SDS011_MSG_OP_SET,
    .src                = SDS011_MSG_SRC_HOST,
    .data.dev_id.new_id = new_id,
  }, cb);
}

sds011_err_t sds011_set_reporting_mode_active(uint16_t dev_id, sds011_cb_t cb) {
  return send_msg(&(sds011_msg_t) {
    .dev_id               = dev_id,
    .type                 = SDS011_MSG_TYPE_REP_MODE,
    .op                   = SDS011_MSG_OP_SET,
    .src                  = SDS011_MSG_SRC_HOST,
    .data.rep_mode.value  = SDS011_REP_MODE_ACTIVE,
  }, cb);
}

sds011_err_t sds011_set_reporting_mode_query(uint16_t dev_id, sds011_cb_t cb) {
  return send_msg(&(sds011_msg_t) {
    .dev_id               = dev_id,
    .type                 = SDS011_MSG_TYPE_REP_MODE,
    .op                   = SDS011_MSG_OP_SET,
    .src                  = SDS011_MSG_SRC_HOST,
    .data.rep_mode.value  = SDS011_REP_MODE_QUERY,
  }, cb);
}

sds011_err_t sds011_get_reporting_mode(uint16_t dev_id, sds011_cb_t cb) {
  return send_msg(&(sds011_msg_t) {
    .dev_id = dev_id,
    .type   = SDS011_MSG_TYPE_REP_MODE,
    .op     = SDS011_MSG_OP_GET,
    .src    = SDS011_MSG_SRC_HOST,
  }, cb);
}

sds011_err_t sds011_set_sleep_on(uint16_t dev_id, sds011_cb_t cb) {
  return send_msg(&(sds011_msg_t) {
    .dev_id           = dev_id,
    .type             = SDS011_MSG_TYPE_SLEEP,
    .op               = SDS011_MSG_OP_SET,
    .src              = SDS011_MSG_SRC_HOST,
    .data.sleep.value = SDS011_SLEEP_ON,
  }, cb);
}

sds011_err_t sds011_set_sleep_off(uint16_t dev_id, sds011_cb_t cb) {
  return send_msg(&(sds011_msg_t) {
    .dev_id           = dev_id,
    .type             = SDS011_MSG_TYPE_SLEEP,
    .op               = SDS011_MSG_OP_SET,
    .src              = SDS011_MSG_SRC_HOST,
    .data.sleep.value = SDS011_SLEEP_OFF,
  }, cb);
}

sds011_err_t sds011_get_sleep(uint16_t dev_id, sds011_cb_t cb) {
  return send_msg(&(sds011_msg_t) {
    .dev_id = dev_id,
    .type   = SDS011_MSG_TYPE_SLEEP,
    .op     = SDS011_MSG_OP_GET,
    .src    = SDS011_MSG_SRC_HOST,
  }, cb);
}

sds011_err_t sds011_set_op_mode_continous(uint16_t dev_id, sds011_cb_t cb) {
  return send_msg(&(sds011_msg_t) {
    .dev_id                 = dev_id,
    .type                   = SDS011_MSG_TYPE_OP_MODE,
    .op                     = SDS011_MSG_OP_SET,
    .src                    = SDS011_MSG_SRC_HOST,
    .data.op_mode.mode      = SDS011_OP_MODE_CONTINOUS,
    .data.op_mode.interval  = 0,
  }, cb);
}

sds011_err_t sds011_set_op_mode_periodic(uint16_t dev_id, uint8_t ival, sds011_cb_t cb) {
  return send_msg(&(sds011_msg_t) {
    .dev_id                 = dev_id,
    .type                   = SDS011_MSG_TYPE_OP_MODE,
    .op                     = SDS011_MSG_OP_SET,
    .src                    = SDS011_MSG_SRC_HOST,
    .data.op_mode.mode      = SDS011_OP_MODE_INTERVAL,
    .data.op_mode.interval  = ival,
  }, cb);
}

sds011_err_t sds011_get_op_mode(uint16_t dev_id, sds011_cb_t cb) {
  return send_msg(&(sds011_msg_t) {
    .dev_id = dev_id,
    .type   = SDS011_MSG_TYPE_OP_MODE,
    .op     = SDS011_MSG_OP_GET,
    .src    = SDS011_MSG_SRC_HOST,
  }, cb);
}

sds011_err_t sds011_get_fw_ver(uint16_t dev_id, sds011_cb_t cb) {
  return send_msg(&(sds011_msg_t) {
    .dev_id = dev_id,
    .type   = SDS011_MSG_TYPE_FW_VER,
    .op     = SDS011_MSG_OP_GET,
    .src    = SDS011_MSG_SRC_HOST,
  }, cb);
}

static sds011_err_t process_byte(uint8_t byte);

sds011_err_t sds011_process(void) {
  process_byte(0);
  return 0;
}

static sds011_err_t confirm(sds011_err_t err, sds011_msg_t const *msg);
static void on_message(sds011_msg_t const *msg);

static sds011_err_t process_byte(uint8_t byte) {
  sds011_msg_t msg;
  sds011_parser_res_t res = sds011_parser_parse(&_parser, byte);

  switch (res) {
    case SDS011_PARSER_RES_RUNNING:
      return SDS011_OK;
    case SDS011_PARSER_RES_ERROR:
      return confirm(sds011_parser_get_error(&_parser), NULL);
    case SDS011_PARSER_RES_READY:
      sds011_parser_get_msg(&_parser, &msg);
      on_message(&msg);
      break;
  }
  return SDS011_OK;
}

static sds011_err_t confirm(sds011_err_t err, sds011_msg_t const *msg) {
  if (err == SDS011_OK && msg == NULL) {
    return SDS011_ERR_INVALID_PARAM;
  }
  if (err != SDS011_OK && msg != NULL) {
    return SDS011_ERR_INVALID_PARAM;
  }

  if (err != SDS011_OK) {
    if (_cb) { _cb(err, NULL); }
    _cb = NULL;
    _cb_msg_type = 0;
  } else if (msg->type == _cb_msg_type) {
    if (_cb) { _cb(err, msg); }
    _cb = NULL;
    _cb_msg_type = 0;
  }

  return err;
}

static void on_message(sds011_msg_t const *msg) {
  if (msg->type == SDS011_MSG_TYPE_DATA) {
    if (_init.on_sample) {
      _init.on_sample(msg->data.sample);
    }
  }

  confirm(SDS011_OK, msg);
}

bool send_buffer(uint8_t *buf, size_t size);

sds011_err_t send_msg(sds011_msg_t const *msg, sds011_cb_t cb) {
  static uint8_t buffer[SDS011_QUERY_PACKET_SIZE];

  // TODO validate busy
  _cb = cb;
  _cb_msg_type = msg->type;

  size_t bytes;
  bytes = sds011_builder_build(msg, buffer, sizeof(buffer));
  if (bytes == 0) {
    return sds011_builder_get_error();
  }
  if (send_buffer(buffer, bytes) == false) {
    return SDS011_ERR_SEND_DATA;
  }

  // TODO implement blocking version

  return SDS011_OK;
}

bool send_buffer(uint8_t *buf, size_t size) {
  // TODO implement + timeout
  return true;
}
