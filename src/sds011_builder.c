#include "sds011_builder.h"

#define MSB(v) (((v) & 0xFF00) >> 8)
#define LSB(v) (((v) & 0x00FF) >> 0)

static size_t build_rep_mode(sds011_msg_t const *msg, uint8_t *buf, size_t size);
static size_t build_sample(sds011_msg_t const *msg, uint8_t *buf, size_t size);
static size_t build_dev_id(sds011_msg_t const *msg, uint8_t *buf, size_t size);
static size_t build_sleep(sds011_msg_t const *msg, uint8_t *buf, size_t size);
static size_t build_fw_ver(sds011_msg_t const *msg, uint8_t *buf, size_t size);
static size_t build_op_mode(sds011_msg_t const *msg, uint8_t *buf, size_t size);

static size_t (*_builder[SDS011_MSG_TYPE_COUNT])(sds011_msg_t const *msg, uint8_t *buf, size_t size) = {
  NULL, // reserved
  NULL, // reserved
  build_rep_mode,
  NULL, // reserved
  build_sample,
  build_dev_id,
  build_sleep,
  build_fw_ver,
  build_op_mode,
};

size_t sds011_builder_build(sds011_msg_t const *msg, uint8_t *buf, size_t size) {
  if (msg == NULL || buf == NULL) { return 0; }
  if (msg->src == SDS011_MSG_SRC_HOST   && size < SDS011_QUERY_PACKET_SIZE) {
    return 0;
  }
  if (msg->src == SDS011_MSG_SRC_SENSOR && size < SDS011_REPLY_PACKET_SIZE) {
    return 0;
  }
  if (msg->type >= SDS011_MSG_TYPE_COUNT) {
    return 0;
  }
  if (_builder[msg->type] == NULL) {
    return 0;
  }

  memset(buf, 0, size);
  return _builder[msg->type](msg, buf, size);
}

static size_t build_rep_mode(sds011_msg_t const *msg, uint8_t *buf, size_t size) {
  uint8_t crc = 0;

  if (msg->src == SDS011_MSG_SRC_HOST) {
    buf[0]  = SDS011_FRAME_BEG;
    buf[1]  = SDS011_CMD_QUERY;
    buf[2]  = SDS011_MSG_TYPE_REP_MODE; crc += buf[2];
    buf[3]  = msg->op;                  crc += buf[3];
    buf[4]  = msg->data.rep_mode.value; crc += buf[4];
    buf[15] = MSB(msg->dev_id);         crc += buf[15];
    buf[16] = LSB(msg->dev_id);         crc += buf[16];
    buf[17] = crc;
    buf[18] = SDS011_FRAME_END;
    return SDS011_QUERY_PACKET_SIZE;
  }
  if (msg->src == SDS011_MSG_SRC_SENSOR) {
    buf[0] = SDS011_FRAME_BEG;
    buf[1] = SDS011_CMD_REPLY;
    buf[2] = SDS011_MSG_TYPE_REP_MODE; crc += buf[2];
    buf[3] = msg->op;                  crc += buf[3];
    buf[4] = msg->data.rep_mode.value; crc += buf[4];
    buf[6] = MSB(msg->dev_id);         crc += buf[6];
    buf[7] = LSB(msg->dev_id);         crc += buf[7];
    buf[8] = crc;
    buf[9] = SDS011_FRAME_END;
    return SDS011_REPLY_PACKET_SIZE;
  }
  return 0;
}

static size_t build_sample(sds011_msg_t const *msg, uint8_t *buf, size_t size) {
  uint8_t crc = 0;

  if (msg->src == SDS011_MSG_SRC_HOST) {
    buf[0]  = SDS011_FRAME_BEG;
    buf[1]  = SDS011_CMD_QUERY;
    buf[2]  = SDS011_MSG_TYPE_DATA; crc += buf[2];
    buf[15] = MSB(msg->dev_id);     crc += buf[15];
    buf[16] = LSB(msg->dev_id);     crc += buf[16];
    buf[17] = crc;
    buf[18] = SDS011_FRAME_END;
    return SDS011_QUERY_PACKET_SIZE;
  }
  if (msg->src == SDS011_MSG_SRC_SENSOR) {
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
  return 0;
}

static size_t build_dev_id(sds011_msg_t const *msg, uint8_t *buf, size_t size) {
  uint8_t crc = 0;

  if (msg->src == SDS011_MSG_SRC_HOST) {
    buf[0]  = SDS011_FRAME_BEG;
    buf[1]  = SDS011_CMD_QUERY;
    buf[2]  = SDS011_MSG_TYPE_DEV_ID;       crc += buf[2];
    buf[13] = MSB(msg->data.dev_id.new_id); crc += buf[13];
    buf[14] = LSB(msg->data.dev_id.new_id); crc += buf[14];
    buf[15] = MSB(msg->dev_id);             crc += buf[15];
    buf[16] = LSB(msg->dev_id);             crc += buf[16];
    buf[17] = crc;
    buf[18] = SDS011_FRAME_END;
    return SDS011_QUERY_PACKET_SIZE;
  }
  if (msg->src == SDS011_MSG_SRC_SENSOR) {
    buf[0] = SDS011_FRAME_BEG;
    buf[1] = SDS011_CMD_REPLY;
    buf[2]  = SDS011_MSG_TYPE_DEV_ID; crc += buf[2];
    buf[6] = MSB(msg->dev_id);        crc += buf[6];
    buf[7] = LSB(msg->dev_id);        crc += buf[7];
    buf[8] = crc;
    buf[9] = SDS011_FRAME_END;
    return SDS011_REPLY_PACKET_SIZE;
  }
  return 0;
}

static size_t build_sleep(sds011_msg_t const *msg, uint8_t *buf, size_t size) {
  return 0;
}

static size_t build_fw_ver(sds011_msg_t const *msg, uint8_t *buf, size_t size) {
  uint8_t crc = 0;

  if (msg->src == SDS011_MSG_SRC_HOST) {
    buf[0]  = SDS011_FRAME_BEG;
    buf[1]  = SDS011_CMD_QUERY;
    buf[2]  = SDS011_MSG_TYPE_FW_VER;       crc += buf[2];
    buf[15] = MSB(msg->dev_id);             crc += buf[15];
    buf[16] = LSB(msg->dev_id);             crc += buf[16];
    buf[17] = crc;
    buf[18] = SDS011_FRAME_END;
    return SDS011_QUERY_PACKET_SIZE;
  }
  if (msg->src == SDS011_MSG_SRC_SENSOR) {
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
  return 0;
}

static size_t build_op_mode(sds011_msg_t const *msg, uint8_t *buf, size_t size) {
  return 0;
}
