
#include <stdint.h>

#include "sds011.h"

sds011_err_t sds011_init(sds011_t *self, sds011_init_t const *init) {
  if (self == NULL) { return SDS011_ERR_INVALID_PARAM; }
  if (init == NULL) { return SDS011_ERR_INVALID_PARAM; }
  if (init->millis == NULL) { return SDS011_ERR_INVALID_PARAM; }
  if (init->serial.bytes_available == NULL) {
    return SDS011_ERR_INVALID_PARAM;
  }
  if (init->serial.read_byte == NULL) {
    return SDS011_ERR_INVALID_PARAM;
  }
  if (init->serial.send_byte == NULL) {
    return SDS011_ERR_INVALID_PARAM;
  }

  sds011_parser_init(&self->parser);
  self->cfg = *init;
  self->req = (sds011_query_req_t) {
    .msg_type   = 0,
    .start_time = 0,
    .cb         = NULL,
  };
  self->on_sample = (sds011_on_sample_t) {
    .callback = NULL,
    .user_data = NULL,
  };
  return SDS011_OK;
}

sds011_err_t sds011_set_sample_callback(sds011_t *self, sds011_sample_cb_t cb, void *user_data) {
  if (self == NULL) { return SDS011_ERR_INVALID_PARAM; }

  self->on_sample = (sds011_on_sample_t) {
    .callback = cb,
    .user_data = user_data,
  };

  return SDS011_OK;
}

static sds011_err_t send_msg(sds011_t *self, sds011_msg_t const *msg, sds011_cb_t cb);

sds011_err_t sds011_query_data(sds011_t *self, uint16_t dev_id, sds011_cb_t cb) {
  return send_msg(self, &(sds011_msg_t) {
    .dev_id = dev_id,
    .type   = SDS011_MSG_TYPE_DATA,
    .op     = SDS011_MSG_OP_GET,
    .src    = SDS011_MSG_SRC_HOST
  }, cb);
}

sds011_err_t sds011_set_device_id(sds011_t *self, uint16_t dev_id, uint16_t new_id, sds011_cb_t cb) {
  return send_msg(self, &(sds011_msg_t) {
    .dev_id             = dev_id,
    .type               = SDS011_MSG_TYPE_DEV_ID,
    .op                 = SDS011_MSG_OP_SET,
    .src                = SDS011_MSG_SRC_HOST,
    .data.dev_id.new_id = new_id,
  }, cb);
}

sds011_err_t sds011_set_reporting_mode_active(sds011_t *self, uint16_t dev_id, sds011_cb_t cb) {
  return send_msg(self, &(sds011_msg_t) {
    .dev_id               = dev_id,
    .type                 = SDS011_MSG_TYPE_REP_MODE,
    .op                   = SDS011_MSG_OP_SET,
    .src                  = SDS011_MSG_SRC_HOST,
    .data.rep_mode.value  = SDS011_REP_MODE_ACTIVE,
  }, cb);
}

sds011_err_t sds011_set_reporting_mode_query(sds011_t *self, uint16_t dev_id, sds011_cb_t cb) {
  return send_msg(self, &(sds011_msg_t) {
    .dev_id               = dev_id,
    .type                 = SDS011_MSG_TYPE_REP_MODE,
    .op                   = SDS011_MSG_OP_SET,
    .src                  = SDS011_MSG_SRC_HOST,
    .data.rep_mode.value  = SDS011_REP_MODE_QUERY,
  }, cb);
}

sds011_err_t sds011_get_reporting_mode(sds011_t *self, uint16_t dev_id, sds011_cb_t cb) {
  return send_msg(self, &(sds011_msg_t) {
    .dev_id = dev_id,
    .type   = SDS011_MSG_TYPE_REP_MODE,
    .op     = SDS011_MSG_OP_GET,
    .src    = SDS011_MSG_SRC_HOST,
  }, cb);
}

sds011_err_t sds011_set_sleep_on(sds011_t *self, uint16_t dev_id, sds011_cb_t cb) {
  return send_msg(self, &(sds011_msg_t) {
    .dev_id           = dev_id,
    .type             = SDS011_MSG_TYPE_SLEEP,
    .op               = SDS011_MSG_OP_SET,
    .src              = SDS011_MSG_SRC_HOST,
    .data.sleep.value = SDS011_SLEEP_ON,
  }, cb);
}

sds011_err_t sds011_set_sleep_off(sds011_t *self, uint16_t dev_id, sds011_cb_t cb) {
  return send_msg(self, &(sds011_msg_t) {
    .dev_id           = dev_id,
    .type             = SDS011_MSG_TYPE_SLEEP,
    .op               = SDS011_MSG_OP_SET,
    .src              = SDS011_MSG_SRC_HOST,
    .data.sleep.value = SDS011_SLEEP_OFF,
  }, cb);
}

sds011_err_t sds011_get_sleep(sds011_t *self, uint16_t dev_id, sds011_cb_t cb) {
  return send_msg(self, &(sds011_msg_t) {
    .dev_id = dev_id,
    .type   = SDS011_MSG_TYPE_SLEEP,
    .op     = SDS011_MSG_OP_GET,
    .src    = SDS011_MSG_SRC_HOST,
  }, cb);
}

sds011_err_t sds011_set_op_mode_continous(sds011_t *self, uint16_t dev_id, sds011_cb_t cb) {
  return send_msg(self, &(sds011_msg_t) {
    .dev_id                 = dev_id,
    .type                   = SDS011_MSG_TYPE_OP_MODE,
    .op                     = SDS011_MSG_OP_SET,
    .src                    = SDS011_MSG_SRC_HOST,
    .data.op_mode.mode      = SDS011_OP_MODE_CONTINOUS,
    .data.op_mode.interval  = 0,
  }, cb);
}

sds011_err_t sds011_set_op_mode_periodic(sds011_t *self, uint16_t dev_id, uint8_t ival, sds011_cb_t cb) {
  return send_msg(self, &(sds011_msg_t) {
    .dev_id                 = dev_id,
    .type                   = SDS011_MSG_TYPE_OP_MODE,
    .op                     = SDS011_MSG_OP_SET,
    .src                    = SDS011_MSG_SRC_HOST,
    .data.op_mode.mode      = SDS011_OP_MODE_INTERVAL,
    .data.op_mode.interval  = ival,
  }, cb);
}

sds011_err_t sds011_get_op_mode(sds011_t *self, uint16_t dev_id, sds011_cb_t cb) {
  return send_msg(self, &(sds011_msg_t) {
    .dev_id = dev_id,
    .type   = SDS011_MSG_TYPE_OP_MODE,
    .op     = SDS011_MSG_OP_GET,
    .src    = SDS011_MSG_SRC_HOST,
  }, cb);
}

sds011_err_t sds011_get_fw_ver(sds011_t *self, uint16_t dev_id, sds011_cb_t cb) {
  return send_msg(self, &(sds011_msg_t) {
    .dev_id = dev_id,
    .type   = SDS011_MSG_TYPE_FW_VER,
    .op     = SDS011_MSG_OP_GET,
    .src    = SDS011_MSG_SRC_HOST,
  }, cb);
}

static sds011_err_t process_byte(sds011_t *self, uint8_t byte);

sds011_err_t sds011_process(sds011_t *self) {
  sds011_err_t err_code = SDS011_OK;
  while (self->cfg.serial.bytes_available(self->cfg.serial.user_data) > 0) {
    uint8_t byte = self->cfg.serial.read_byte(self->cfg.serial.user_data);
    if ((err_code = process_byte(self, byte)) != SDS011_OK) {
      return err_code;
    }
  }
  return SDS011_OK;
}

static sds011_err_t confirm(sds011_t *self, sds011_err_t err, sds011_msg_t const *msg);
static void on_message(sds011_t *self, sds011_msg_t const *msg);
static bool is_timeout(sds011_t *self, uint32_t beg, uint32_t timeout);

static sds011_err_t process_byte(sds011_t *self, uint8_t byte) {
  sds011_msg_t msg;
  sds011_parser_res_t res = sds011_parser_parse(&self->parser, byte);

  switch (res) {
    case SDS011_PARSER_RES_RUNNING:
      return SDS011_OK;
    case SDS011_PARSER_RES_ERROR:
      return confirm(self, sds011_parser_get_error(&self->parser), NULL);
    case SDS011_PARSER_RES_READY:
      sds011_parser_get_msg(&self->parser, &msg);
      on_message(self, &msg);
      break;
  }

  // check query timeout
  if (self->req.cb != NULL) {
    if (is_timeout(self, self->req.start_time, self->cfg.msg_timeout)) {
      confirm(self, SDS011_ERR_TIMEOUT, NULL);
    }
  }

  return SDS011_OK;
}

static bool is_timeout(sds011_t *self, uint32_t beg, uint32_t timeout) {
  if (timeout == 0) {
    return false;
  }
  return (self->cfg.millis() - beg) > timeout;
}

static sds011_err_t confirm(sds011_t *self, sds011_err_t err, sds011_msg_t const *msg) {
  if (err == SDS011_OK && msg == NULL) {
    return SDS011_ERR_INVALID_PARAM;
  }
  if (err != SDS011_OK && msg != NULL) {
    return SDS011_ERR_INVALID_PARAM;
  }

  if (err != SDS011_OK) {
    if (self->req.cb) { self->req.cb(err, NULL); }
    self->req.cb = NULL;
    self->req.msg_type = 0;
  } else if (msg->type == self->req.msg_type) {
    if (self->req.cb) { self->req.cb(err, msg); }
    self->req.cb = NULL;
    self->req.msg_type = 0;
  }

  return err;
}

static void on_message(sds011_t *self, sds011_msg_t const *msg) {
  if (msg->type == SDS011_MSG_TYPE_DATA) {
    if (self->on_sample.callback) {
      self->on_sample.callback(msg, self->on_sample.user_data);
    }
  }

  confirm(self, SDS011_OK, msg);
}

static bool send_buffer(sds011_t *self, uint8_t *buf, size_t size);

static sds011_err_t send_msg(sds011_t *self, sds011_msg_t const *msg, sds011_cb_t cb) {
  static uint8_t buffer[SDS011_QUERY_PACKET_SIZE];
  size_t bytes;

  if (self == NULL) {
    return SDS011_ERR_INVALID_PARAM;
  }

  if (self->req.cb != NULL || self->req.msg_type != 0) {
    return SDS011_ERR_BUSY;
  }

  self->req.cb         = cb;
  self->req.msg_type   = msg->type;
  self->req.start_time = self->cfg.millis();

  if ((bytes = sds011_builder_build(msg, buffer, sizeof(buffer))) == 0) {
    return sds011_builder_get_error();
  }

  if (send_buffer(self, buffer, bytes) == false) {
    return SDS011_ERR_SEND_DATA;
  }

  return SDS011_OK;
}

static bool send_buffer(sds011_t *self, uint8_t *buf, size_t size) {
  uint32_t beg = self->cfg.millis();

  for (size_t i = 0; i < size; i++) {
    while (self->cfg.serial.send_byte(buf[i], self->cfg.serial.user_data) == false) {
      if (is_timeout(self, beg, self->cfg.msg_send_timeout)) {
        return false;
      }
    }
  }
  return true;
}
