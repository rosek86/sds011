
#include <stdint.h>

#include "sds011.h"

static bool init_req_queue(sds011_t *self);

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

  self->cfg = *init;
  sds011_parser_init(&self->parser);

  memset(&self->on_sample, 0, sizeof(self->on_sample));

  if (init_req_queue(self) == false) {
    return SDS011_ERR_INVALID_PARAM;
  }

  return SDS011_OK;
}

static bool init_req_queue(sds011_t *self) {
  sds011_requests_t *req = &self->req;

  memset(&req->mem, 0, sizeof(req->mem));
  memset(&req->active, 0, sizeof(req->active));
  req->status = SDS011_REQ_STATUS_IDLE;
  req->critical = false;
  req->retry = 0;
  req->start_time = 0;

  return sds011_fifo_init(&req->queue, sizeof(sds011_request_t), &req->mem, sizeof(req->mem));
}

sds011_err_t sds011_set_sample_callback(sds011_t *self, sds011_on_sample_t cb) {
  if (self == NULL) { return SDS011_ERR_INVALID_PARAM; }
  self->on_sample = cb;
  return SDS011_OK;
}

static sds011_err_t push_msg(sds011_t *self, sds011_msg_t const *msg, sds011_cb_t cb);

sds011_err_t sds011_query_data(sds011_t *self, uint16_t dev_id, sds011_cb_t cb) {
  return push_msg(self, &(sds011_msg_t) {
    .dev_id = dev_id,
    .type   = SDS011_MSG_TYPE_DATA,
    .op     = SDS011_MSG_OP_GET,
    .src    = SDS011_MSG_SRC_HOST
  }, cb);
}

static void confirm(sds011_cb_t *cb, sds011_err_t err, sds011_msg_t const *msg);

static sds011_err_t push_msg(sds011_t *self, sds011_msg_t const *msg, sds011_cb_t cb) {
  if (self == NULL) { return SDS011_ERR_INVALID_PARAM; }

  if (sds011_fifo_push(&self->req.queue, &(sds011_request_t) {
    .msg   = *msg,
    .cb    = cb,
  }) == false) {
    confirm(&cb, SDS011_ERR_BUSY, NULL);
    return SDS011_ERR_BUSY;
  }

  return SDS011_OK;
}

static void confirm(sds011_cb_t *cb, sds011_err_t err, sds011_msg_t const *msg) {
  if (cb->callback) {
    cb->callback(err, msg, cb->user_data);
  }
}

sds011_err_t sds011_set_device_id(sds011_t *self, uint16_t dev_id, uint16_t new_id, sds011_cb_t cb) {
  return push_msg(self, &(sds011_msg_t) {
    .dev_id          = dev_id,
    .type            = SDS011_MSG_TYPE_DEV_ID,
    .op              = SDS011_MSG_OP_SET,
    .src             = SDS011_MSG_SRC_HOST,
    .data.new_dev_id = new_id,
  }, cb);
}

sds011_err_t sds011_set_reporting_mode_active(sds011_t *self, uint16_t dev_id, sds011_cb_t cb) {
  return push_msg(self, &(sds011_msg_t) {
    .dev_id        = dev_id,
    .type          = SDS011_MSG_TYPE_REP_MODE,
    .op            = SDS011_MSG_OP_SET,
    .src           = SDS011_MSG_SRC_HOST,
    .data.rep_mode = SDS011_REP_MODE_ACTIVE,
  }, cb);
}

sds011_err_t sds011_set_reporting_mode_query(sds011_t *self, uint16_t dev_id, sds011_cb_t cb) {
  return push_msg(self, &(sds011_msg_t) {
    .dev_id        = dev_id,
    .type          = SDS011_MSG_TYPE_REP_MODE,
    .op            = SDS011_MSG_OP_SET,
    .src           = SDS011_MSG_SRC_HOST,
    .data.rep_mode = SDS011_REP_MODE_QUERY,
  }, cb);
}

sds011_err_t sds011_get_reporting_mode(sds011_t *self, uint16_t dev_id, sds011_cb_t cb) {
  return push_msg(self, &(sds011_msg_t) {
    .dev_id = dev_id,
    .type   = SDS011_MSG_TYPE_REP_MODE,
    .op     = SDS011_MSG_OP_GET,
    .src    = SDS011_MSG_SRC_HOST,
  }, cb);
}

sds011_err_t sds011_set_sleep_on(sds011_t *self, uint16_t dev_id, sds011_cb_t cb) {
  return push_msg(self, &(sds011_msg_t) {
    .dev_id     = dev_id,
    .type       = SDS011_MSG_TYPE_SLEEP,
    .op         = SDS011_MSG_OP_SET,
    .src        = SDS011_MSG_SRC_HOST,
    .data.sleep = SDS011_SLEEP_ON,
  }, cb);
}

sds011_err_t sds011_set_sleep_off(sds011_t *self, uint16_t dev_id, sds011_cb_t cb) {
  return push_msg(self, &(sds011_msg_t) {
    .dev_id     = dev_id,
    .type       = SDS011_MSG_TYPE_SLEEP,
    .op         = SDS011_MSG_OP_SET,
    .src        = SDS011_MSG_SRC_HOST,
    .data.sleep = SDS011_SLEEP_OFF,
  }, cb);
}

sds011_err_t sds011_get_sleep(sds011_t *self, uint16_t dev_id, sds011_cb_t cb) {
  return push_msg(self, &(sds011_msg_t) {
    .dev_id = dev_id,
    .type   = SDS011_MSG_TYPE_SLEEP,
    .op     = SDS011_MSG_OP_GET,
    .src    = SDS011_MSG_SRC_HOST,
  }, cb);
}

sds011_err_t sds011_set_op_mode_continous(sds011_t *self, uint16_t dev_id, sds011_cb_t cb) {
  return push_msg(self, &(sds011_msg_t) {
    .dev_id                 = dev_id,
    .type                   = SDS011_MSG_TYPE_OP_MODE,
    .op                     = SDS011_MSG_OP_SET,
    .src                    = SDS011_MSG_SRC_HOST,
    .data.op_mode.mode      = SDS011_OP_MODE_CONTINOUS,
    .data.op_mode.interval  = 0,
  }, cb);
}

sds011_err_t sds011_set_op_mode_periodic(sds011_t *self, uint16_t dev_id, uint8_t ival, sds011_cb_t cb) {
  return push_msg(self, &(sds011_msg_t) {
    .dev_id                 = dev_id,
    .type                   = SDS011_MSG_TYPE_OP_MODE,
    .op                     = SDS011_MSG_OP_SET,
    .src                    = SDS011_MSG_SRC_HOST,
    .data.op_mode.mode      = SDS011_OP_MODE_INTERVAL,
    .data.op_mode.interval  = ival,
  }, cb);
}

sds011_err_t sds011_get_op_mode(sds011_t *self, uint16_t dev_id, sds011_cb_t cb) {
  return push_msg(self, &(sds011_msg_t) {
    .dev_id = dev_id,
    .type   = SDS011_MSG_TYPE_OP_MODE,
    .op     = SDS011_MSG_OP_GET,
    .src    = SDS011_MSG_SRC_HOST,
  }, cb);
}

sds011_err_t sds011_get_fw_ver(sds011_t *self, uint16_t dev_id, sds011_cb_t cb) {
  return push_msg(self, &(sds011_msg_t) {
    .dev_id = dev_id,
    .type   = SDS011_MSG_TYPE_FW_VER,
    .op     = SDS011_MSG_OP_GET,
    .src    = SDS011_MSG_SRC_HOST,
  }, cb);
}

static bool is_timeout(sds011_t *self, uint32_t beg, uint32_t timeout);
static void send_active_msg(sds011_t *self);
static sds011_err_t process_byte(sds011_t *self, uint8_t byte);

sds011_err_t sds011_process(sds011_t *self) {
  sds011_err_t err_code = SDS011_OK;

  while (self->cfg.serial.bytes_available(self->cfg.serial.user_data) > 0) {
    uint8_t byte = self->cfg.serial.read_byte(self->cfg.serial.user_data);
    if ((err_code = process_byte(self, byte)) != SDS011_OK) {
      break;
    }
  }

  if (self->req.status == SDS011_REQ_STATUS_RUNNING) {
    if (is_timeout(self, self->req.start_time, self->cfg.msg_timeout)) {
      self->req.status = SDS011_REQ_STATUS_FAILURE;
      self->req.err = SDS011_ERR_TIMEOUT;
    }
  }

  if (self->req.status == SDS011_REQ_STATUS_SUCCESS) {
    confirm(&self->req.active.cb, SDS011_OK, &self->req.msg);
    self->req.status = SDS011_REQ_STATUS_IDLE;
  }

  if (self->req.status == SDS011_REQ_STATUS_FAILURE) {
    if (self->req.critical == true) {
      confirm(&self->req.active.cb, self->req.err, NULL);
      self->req.status = SDS011_REQ_STATUS_IDLE;
    } else if (++self->req.retry >= self->cfg.retries) {
      confirm(&self->req.active.cb, self->req.err, NULL);
      self->req.status = SDS011_REQ_STATUS_IDLE;
    } else {
      send_active_msg(self);
    }
  }

  if (self->req.status == SDS011_REQ_STATUS_IDLE) {
    if (sds011_fifo_pop(&self->req.queue, &self->req.active) == true) {
      self->req.retry = 0;
      send_active_msg(self);
    }
  }

  return err_code;
}

static bool is_timeout(sds011_t *self, uint32_t beg, uint32_t timeout) {
  if (timeout == 0) {
    return false;
  }
  return (self->cfg.millis() - beg) > timeout;
}

static bool send_buffer(sds011_t *self, uint8_t *buf, size_t size);

static void send_active_msg(sds011_t *self) {
  static uint8_t buffer[SDS011_QUERY_PACKET_SIZE];
  size_t bytes;
  sds011_msg_t *msg = &self->req.active.msg;

  self->req.status = SDS011_REQ_STATUS_RUNNING;
  self->req.critical = false;
  self->req.start_time = self->cfg.millis();

  if ((bytes = sds011_builder_build(msg, buffer, sizeof(buffer))) == 0) {
    self->req.err = sds011_builder_get_error();
    self->req.status = SDS011_REQ_STATUS_FAILURE;
    self->req.critical = true;
    return;
  }

  if (send_buffer(self, buffer, bytes) == false) {
    self->req.err = SDS011_ERR_SEND_DATA;
    self->req.status = SDS011_REQ_STATUS_FAILURE;
    return;
  }
}

static bool send_buffer(sds011_t *self, uint8_t *buf, size_t size) {
  for (size_t i = 0; i < size; i++) {
    while (self->cfg.serial.send_byte(buf[i], self->cfg.serial.user_data) == false) {
      if (is_timeout(self, self->req.start_time, self->cfg.msg_timeout)) {
        return false;
      }
    }
  }
  return true;
}

static void on_message(sds011_t *self, sds011_msg_t const *msg);

static sds011_err_t process_byte(sds011_t *self, uint8_t byte) {
  sds011_msg_t msg;
  sds011_parser_res_t res = sds011_parser_parse(&self->parser, byte);

  switch (res) {
    case SDS011_PARSER_RES_RUNNING:
      return SDS011_OK;
    case SDS011_PARSER_RES_ERROR:
      return sds011_parser_get_error(&self->parser);
    case SDS011_PARSER_RES_READY:
      sds011_parser_get_msg(&self->parser, &msg);
      on_message(self, &msg);
      break;
  }

  return SDS011_OK;
}

static bool is_callback_for_msg(sds011_t *self, sds011_msg_t const *msg);

static void on_message(sds011_t *self, sds011_msg_t const *msg) {
  if (msg->type == SDS011_MSG_TYPE_DATA) {
    if (self->on_sample.callback) {
      self->on_sample.callback(msg, self->on_sample.user_data);
    }
  }

  if (is_callback_for_msg(self, msg) == false) {
    return;
  }

  if (sds011_validator_validate(&self->req.active.msg, msg) == false) {
    self->req.status = SDS011_REQ_STATUS_FAILURE;
    self->req.err = SDS011_ERR_INVALID_REPLY;
  } else {
    self->req.status = SDS011_REQ_STATUS_SUCCESS;
    self->req.msg = *msg;
  }
}

static bool is_callback_for_msg(sds011_t *self, sds011_msg_t const *msg) {
  if (self->req.status != SDS011_REQ_STATUS_RUNNING) { return false; }

  if (self->req.active.msg.type != msg->type) { return false; }
  if (self->req.active.msg.op   != msg->op  ) { return false; }

  if (msg->type == SDS011_MSG_TYPE_DEV_ID) {
    if (self->req.active.msg.data.new_dev_id != msg->dev_id) {
      return false;
    }
  }

  if (self->req.active.msg.dev_id != 0xFFFF) {
    if (self->req.active.msg.dev_id != msg->dev_id) {
      return false;
    }
  }

  return true;
}
