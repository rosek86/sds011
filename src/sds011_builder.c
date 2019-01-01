#include "sds011_builder.h"

#define MSB(v) (((v) & 0xFF00) >> 8)
#define LSB(v) (((v) & 0x00FF) >> 0)

static sds011_err_t _error = SDS011_OK;

static size_t build_sens_rep_mode(sds011_msg_t const *msg, uint8_t *buf, size_t size);
static size_t build_host_rep_mode(sds011_msg_t const *msg, uint8_t *buf, size_t size);
static size_t build_sens_sample  (sds011_msg_t const *msg, uint8_t *buf, size_t size);
static size_t build_host_sample  (sds011_msg_t const *msg, uint8_t *buf, size_t size);
static size_t build_sens_dev_id  (sds011_msg_t const *msg, uint8_t *buf, size_t size);
static size_t build_host_dev_id  (sds011_msg_t const *msg, uint8_t *buf, size_t size);
static size_t build_sens_sleep   (sds011_msg_t const *msg, uint8_t *buf, size_t size);
static size_t build_host_sleep   (sds011_msg_t const *msg, uint8_t *buf, size_t size);
static size_t build_sens_fw_ver  (sds011_msg_t const *msg, uint8_t *buf, size_t size);
static size_t build_host_fw_ver  (sds011_msg_t const *msg, uint8_t *buf, size_t size);
static size_t build_sens_op_mode (sds011_msg_t const *msg, uint8_t *buf, size_t size);
static size_t build_host_op_mode (sds011_msg_t const *msg, uint8_t *buf, size_t size);

static size_t (*_builder_host[2][SDS011_MSG_TYPE_COUNT])(sds011_msg_t const *msg, uint8_t *buf, size_t size) = {
  {
    NULL, // reserved
    NULL, // reserved
    build_sens_rep_mode,
    NULL, // reserved
    build_sens_sample,
    build_sens_dev_id,
    build_sens_sleep,
    build_sens_fw_ver,
    build_sens_op_mode,
  }, {
    NULL, // reserved
    NULL, // reserved
    build_host_rep_mode,
    NULL, // reserved
    build_host_sample,
    build_host_dev_id,
    build_host_sleep,
    build_host_fw_ver,
    build_host_op_mode,
  }
};

size_t sds011_builder_build(sds011_msg_t const *msg, uint8_t *buf, size_t size) {
  if (msg == NULL || buf == NULL) {
    _error = SDS011_ERR_INVALID_PARAM;
    return 0;
  }
  if (msg->src == SDS011_MSG_SRC_HOST && size < SDS011_QUERY_PACKET_SIZE) {
    _error = SDS011_ERR_MEM;
    return 0;
  }
  if (msg->src == SDS011_MSG_SRC_SENSOR && size < SDS011_REPLY_PACKET_SIZE) {
    _error = SDS011_ERR_MEM;
    return 0;
  }
  if (msg->src != SDS011_MSG_SRC_HOST && msg->src != SDS011_MSG_SRC_SENSOR) {
    _error = SDS011_ERR_INVALID_SRC;
    return 0;
  }
  if (msg->type >= SDS011_MSG_TYPE_COUNT) {
    _error = SDS011_ERR_INVALID_MSG_TYPE;
    return 0;
  }
  if (_builder_host[msg->src][msg->type] == NULL) {
    _error = SDS011_ERR_INVALID_MSG_TYPE;
    return 0;
  }

  memset(buf, 0, size);
  return _builder_host[msg->src][msg->type](msg, buf, size);
}

static size_t build_sens_rep_mode(sds011_msg_t const *msg, uint8_t *buf, size_t size) {
  uint8_t crc = 0;
  buf[0] = SDS011_FRAME_BEG;
  buf[1] = SDS011_CMD_REPLY;
  buf[2] = SDS011_MSG_TYPE_REP_MODE; crc += buf[2];
  buf[3] = msg->op;                  crc += buf[3];
  buf[4] = msg->data.rep_mode;       crc += buf[4];
  buf[6] = MSB(msg->dev_id);         crc += buf[6];
  buf[7] = LSB(msg->dev_id);         crc += buf[7];
  buf[8] = crc;
  buf[9] = SDS011_FRAME_END;
  return SDS011_REPLY_PACKET_SIZE;
}

static size_t build_host_rep_mode(sds011_msg_t const *msg, uint8_t *buf, size_t size) {
  uint8_t crc = 0;
  buf[0]  = SDS011_FRAME_BEG;
  buf[1]  = SDS011_CMD_QUERY;
  buf[2]  = SDS011_MSG_TYPE_REP_MODE;   crc += buf[2];
  buf[3]  = msg->op;                    crc += buf[3];
  if (msg->op == SDS011_MSG_OP_SET) {
    buf[4]  = msg->data.rep_mode;       crc += buf[4];
  }
  buf[15] = MSB(msg->dev_id);           crc += buf[15];
  buf[16] = LSB(msg->dev_id);           crc += buf[16];
  buf[17] = crc;
  buf[18] = SDS011_FRAME_END;
  return SDS011_QUERY_PACKET_SIZE;
}

static size_t build_sens_sample(sds011_msg_t const *msg, uint8_t *buf, size_t size) {
  uint8_t crc = 0;
  buf[0] = SDS011_FRAME_BEG;
  buf[1] = SDS011_DAT_REPLY;
  buf[2] = LSB(msg->data.sample.pm2_5); crc += buf[2];
  buf[3] = MSB(msg->data.sample.pm2_5); crc += buf[3];
  buf[4] = LSB(msg->data.sample.pm10);  crc += buf[4];
  buf[5] = MSB(msg->data.sample.pm10);  crc += buf[5];
  buf[6] = MSB(msg->dev_id);            crc += buf[6];
  buf[7] = LSB(msg->dev_id);            crc += buf[7];
  buf[8] = crc;
  buf[9] = SDS011_FRAME_END;
  return SDS011_REPLY_PACKET_SIZE;
}

static size_t build_host_sample(sds011_msg_t const *msg, uint8_t *buf, size_t size) {
  uint8_t crc = 0;
  buf[0]  = SDS011_FRAME_BEG;
  buf[1]  = SDS011_CMD_QUERY;
  buf[2]  = SDS011_MSG_TYPE_DATA; crc += buf[2];
  buf[15] = MSB(msg->dev_id);     crc += buf[15];
  buf[16] = LSB(msg->dev_id);     crc += buf[16];
  buf[17] = crc;
  buf[18] = SDS011_FRAME_END;
  return SDS011_QUERY_PACKET_SIZE;
}

static size_t build_sens_dev_id(sds011_msg_t const *msg, uint8_t *buf, size_t size) {
  uint8_t crc = 0;
  buf[0] = SDS011_FRAME_BEG;
  buf[1] = SDS011_CMD_REPLY;
  buf[2]  = SDS011_MSG_TYPE_DEV_ID; crc += buf[2];
  buf[6] = MSB(msg->dev_id);        crc += buf[6];
  buf[7] = LSB(msg->dev_id);        crc += buf[7];
  buf[8] = crc;
  buf[9] = SDS011_FRAME_END;
  return SDS011_REPLY_PACKET_SIZE;
}

static size_t build_host_dev_id(sds011_msg_t const *msg, uint8_t *buf, size_t size) {
  uint8_t crc = 0;
  buf[0]  = SDS011_FRAME_BEG;
  buf[1]  = SDS011_CMD_QUERY;
  buf[2]  = SDS011_MSG_TYPE_DEV_ID;    crc += buf[2];
  buf[13] = MSB(msg->data.new_dev_id); crc += buf[13];
  buf[14] = LSB(msg->data.new_dev_id); crc += buf[14];
  buf[15] = MSB(msg->dev_id);          crc += buf[15];
  buf[16] = LSB(msg->dev_id);          crc += buf[16];
  buf[17] = crc;
  buf[18] = SDS011_FRAME_END;
  return SDS011_QUERY_PACKET_SIZE;
}

static size_t build_sens_sleep(sds011_msg_t const *msg, uint8_t *buf, size_t size) {
  uint8_t crc = 0;
  buf[0] = SDS011_FRAME_BEG;
  buf[1] = SDS011_CMD_REPLY;
  buf[2] = SDS011_MSG_TYPE_SLEEP;   crc += buf[2];
  buf[3] = msg->op;                 crc += buf[3];
  buf[4] = msg->data.sleep;         crc += buf[4];
  buf[6] = MSB(msg->dev_id);        crc += buf[6];
  buf[7] = LSB(msg->dev_id);        crc += buf[7];
  buf[8] = crc;
  buf[9] = SDS011_FRAME_END;
  return SDS011_REPLY_PACKET_SIZE;
}

static size_t build_host_sleep(sds011_msg_t const *msg, uint8_t *buf, size_t size) {
  uint8_t crc = 0;
  buf[0]  = SDS011_FRAME_BEG;
  buf[1]  = SDS011_CMD_QUERY;
  buf[2]  = SDS011_MSG_TYPE_SLEEP;    crc += buf[2];
  buf[3]  = msg->op;                  crc += buf[3];
  if (msg->op == SDS011_MSG_OP_SET) {
    buf[4]  = msg->data.sleep;        crc += buf[4];
  }
  buf[15] = MSB(msg->dev_id);         crc += buf[15];
  buf[16] = LSB(msg->dev_id);         crc += buf[16];
  buf[17] = crc;
  buf[18] = SDS011_FRAME_END;
  return SDS011_QUERY_PACKET_SIZE;
}

static size_t build_sens_fw_ver(sds011_msg_t const *msg, uint8_t *buf, size_t size) {
  uint8_t crc = 0;
  buf[0] = SDS011_FRAME_BEG;
  buf[1] = SDS011_CMD_REPLY;
  buf[2] = SDS011_MSG_TYPE_FW_VER;  crc += buf[2];
  buf[3] = msg->data.fw_ver.year;   crc += buf[3];
  buf[4] = msg->data.fw_ver.month;  crc += buf[4];
  buf[5] = msg->data.fw_ver.day;    crc += buf[5];
  buf[6] = MSB(msg->dev_id);        crc += buf[6];
  buf[7] = LSB(msg->dev_id);        crc += buf[7];
  buf[8] = crc;
  buf[9] = SDS011_FRAME_END;
  return SDS011_REPLY_PACKET_SIZE;
}

static size_t build_host_fw_ver(sds011_msg_t const *msg, uint8_t *buf, size_t size) {
  uint8_t crc = 0;
  buf[0]  = SDS011_FRAME_BEG;
  buf[1]  = SDS011_CMD_QUERY;
  buf[2]  = SDS011_MSG_TYPE_FW_VER; crc += buf[2];
  buf[15] = MSB(msg->dev_id);       crc += buf[15];
  buf[16] = LSB(msg->dev_id);       crc += buf[16];
  buf[17] = crc;
  buf[18] = SDS011_FRAME_END;
  return SDS011_QUERY_PACKET_SIZE;
}

static size_t build_sens_op_mode(sds011_msg_t const *msg, uint8_t *buf, size_t size) {
  uint8_t crc = 0;
  buf[0] = SDS011_FRAME_BEG;
  buf[1] = SDS011_CMD_REPLY;
  buf[2] = SDS011_MSG_TYPE_OP_MODE;       crc += buf[2];
  buf[3] = msg->op;                       crc += buf[3];

  if (msg->data.op_mode.mode == SDS011_OP_MODE_CONTINOUS) {
    buf[4] = 0;                           crc += buf[4];
  } else {
    buf[4] = msg->data.op_mode.interval;  crc += buf[4];
  }

  buf[6] = MSB(msg->dev_id);              crc += buf[6];
  buf[7] = LSB(msg->dev_id);              crc += buf[7];
  buf[8] = crc;
  buf[9] = SDS011_FRAME_END;
  return SDS011_REPLY_PACKET_SIZE;
}

static size_t build_host_op_mode(sds011_msg_t const *msg, uint8_t *buf, size_t size) {
  uint8_t crc = 0;
  buf[0] = SDS011_FRAME_BEG;
  buf[1] = SDS011_CMD_QUERY;
  buf[2] = SDS011_MSG_TYPE_OP_MODE;         crc += buf[2];
  buf[3] = msg->op;                         crc += buf[3];
  if (msg->op == SDS011_MSG_OP_SET) {
    if (msg->data.op_mode.mode == SDS011_OP_MODE_CONTINOUS) {
      buf[4] = 0;                           crc += buf[4];
    } else {
      buf[4] = msg->data.op_mode.interval;  crc += buf[4];
    }
  }
  buf[15] = MSB(msg->dev_id);               crc += buf[15];
  buf[16] = LSB(msg->dev_id);               crc += buf[16];
  buf[17] = crc;
  buf[18] = SDS011_FRAME_END;
  return SDS011_QUERY_PACKET_SIZE;
}

sds011_err_t sds011_builder_get_error(void) {
  return _error;
}
