#include "sds011.h"

#include <string.h>

// TODO when to clear parser (user or auto)
//      errors during final parsing stage

#define SDS011_FRAME_BEG 0xAA
#define SDS011_FRAME_END 0xAB

#define SDS011_CMD_QUERY 0xB4
#define SDS011_CMD_REPLY 0xC5
#define SDS011_DAT_REPLY 0xC0

#define SDS011_QUERY_DATA_SIZE 15
#define SDS011_REPLY_DATA_SIZE 6

#define VALUE16(hi, lo) (((uint16_t)(hi) << 8) | (lo))

typedef enum {
  STATE_BEG,
  STATE_CMD,
  STATE_DATA,
  STATE_CRC,
  STATE_END,
} state_t;

static uint8_t payload_len_by_cmd(uint8_t cmd);

sds011_parser_res_t sds011_parser_parse(sds011_parser_t *parser, uint8_t byte) {
  switch (parser->state) {
    case STATE_BEG:
      if (byte == SDS011_FRAME_BEG) {
        parser->state++;
      } else {
        sds011_parser_clear(parser);
        parser->error = SDS011_PARSER_ERR_FRAME_BEG;
        return SDS011_PARSER_RES_ERROR;
      }
      break;
    case STATE_CMD:
      if ((parser->data_len = payload_len_by_cmd(byte)) > 0) {
        parser->cmd = byte;
        parser->data_iter = 0;
        parser->state++;
      } else {
        sds011_parser_clear(parser);
        parser->error = SDS011_PARSER_ERR_CMD;
        return SDS011_PARSER_RES_ERROR;
      }
      break;
    case STATE_DATA:
      parser->data[parser->data_iter++] = byte;
      parser->data_crc += byte;

      if (parser->data_iter >= parser->data_len) {
        parser->state++;
      }
      break;
    case STATE_CRC:
      if (parser->data_crc == byte) {
        parser->state++;
      } else {
        sds011_parser_clear(parser);
        parser->error = SDS011_PARSER_ERR_CRC;
        return SDS011_PARSER_RES_ERROR;
      }
      break;
    case STATE_END:
      if (byte == SDS011_FRAME_END) {
        parser->state++;
        return SDS011_PARSER_RES_READY;
      } else {
        sds011_parser_clear(parser);
        parser->error = SDS011_PARSER_ERR_FRAME_END;
        return SDS011_PARSER_RES_ERROR;
      }
      break;
  }

  return SDS011_PARSER_RES_RUNNING;
}

static uint8_t payload_len_by_cmd(uint8_t cmd) {
  if (cmd == SDS011_CMD_QUERY) {
    return SDS011_QUERY_DATA_SIZE;
  }
  if (cmd == SDS011_CMD_REPLY || cmd == SDS011_DAT_REPLY) {
    return SDS011_REPLY_DATA_SIZE;
  }
  return 0;
}

static sds011_msg_type_t get_msg_type(sds011_parser_t const *parser);

// parsers
static bool parse_rep_mode(sds011_parser_t const *parser, sds011_msg_t *msg);
static bool parse_data(sds011_parser_t const *parser, sds011_msg_t *msg);
static bool parse_dev_id(sds011_parser_t const *parser, sds011_msg_t *msg);
static bool parse_sleep(sds011_parser_t const *parser, sds011_msg_t *msg);
static bool parse_fw_ver(sds011_parser_t const *parser, sds011_msg_t *msg);
static bool parse_op_mode(sds011_parser_t const *parser, sds011_msg_t *msg);

static bool (*_msg_parsers[9])(sds011_parser_t const *parser, sds011_msg_t *msg) = {
  NULL,           // reserved
  NULL,           // reserved
  parse_rep_mode,
  NULL,           // reserved
  parse_data,
  parse_dev_id,
  parse_sleep,
  parse_fw_ver,
  parse_op_mode,
};

bool sds011_parser_get_msg(sds011_parser_t const *parser, sds011_msg_t *msg) {
  sds011_msg_type_t type = get_msg_type(parser);

  if (type >= SDS011_MSG_TYPE_COUNT) {
    return false;
  }
  if (_msg_parsers[type] == NULL) {
    return false;
  }

  memset(msg, 0, sizeof(sds011_msg_t));
  return _msg_parsers[type](parser, msg);
}

static sds011_msg_type_t get_msg_type(sds011_parser_t const *parser) {
  if (parser->cmd == SDS011_DAT_REPLY) {
    return SDS011_MSG_TYPE_DATA;
  }
  return parser->data[0];
}

static bool parse_rep_mode(sds011_parser_t const *parser, sds011_msg_t *msg) {
  uint8_t op = parser->data[1];
  uint8_t rm = parser->data[2];

  if (op != SDS011_MSG_OP_GET && op != SDS011_MSG_OP_SET) {
    return false;
  }
  if (rm != SDS011_REP_MODE_ACTIVE && rm != SDS011_REP_MODE_QUERY) {
    return false;
  }
  if (parser->cmd == SDS011_CMD_QUERY) {
    msg->dev_id               = VALUE16(parser->data[13], parser->data[14]);
    msg->type                 = SDS011_MSG_TYPE_REP_MODE;
    msg->op                   = op;
    msg->src                  = SDS011_MSG_SRC_HOST;
    msg->data.rep_mode.value  = rm;
    return true;
  }
  if (parser->cmd == SDS011_CMD_REPLY) {
    msg->dev_id               = VALUE16(parser->data[4], parser->data[5]);
    msg->type                 = SDS011_MSG_TYPE_REP_MODE;
    msg->op                   = op;
    msg->src                  = SDS011_MSG_SRC_SENSOR;
    msg->data.rep_mode.value  = rm;
    return true;
  }
  return false;
}

static bool parse_data(sds011_parser_t const *parser, sds011_msg_t *msg) {
  if (parser->cmd == SDS011_CMD_QUERY) {
    msg->dev_id            = VALUE16(parser->data[13], parser->data[14]);
    msg->type              = SDS011_MSG_TYPE_DATA;
    msg->op                = SDS011_MSG_OP_GET;
    msg->src               = SDS011_MSG_SRC_HOST;
    return true;
  }
  if (parser->cmd == SDS011_DAT_REPLY) {
    msg->dev_id            = VALUE16(parser->data[4], parser->data[5]);
    msg->type              = SDS011_MSG_TYPE_DATA;
    msg->op                = SDS011_MSG_OP_GET;
    msg->src               = SDS011_MSG_SRC_SENSOR;
    msg->data.sample.pm2_5 = VALUE16(parser->data[1], parser->data[0]);
    msg->data.sample.pm10  = VALUE16(parser->data[3], parser->data[2]);
    return true;
  }
  return false;
}

static bool parse_dev_id(sds011_parser_t const *parser, sds011_msg_t *msg) {
  if (parser->cmd == SDS011_CMD_QUERY) {
    msg->dev_id             = VALUE16(parser->data[13], parser->data[14]);
    msg->type               = SDS011_MSG_TYPE_DEV_ID;
    msg->op                 = SDS011_MSG_OP_SET;
    msg->src                = SDS011_MSG_SRC_HOST;
    msg->data.dev_id.new_id = VALUE16(parser->data[11], parser->data[12]);
    return true;
  }
  if (parser->cmd == SDS011_CMD_REPLY) {
    msg->dev_id            = VALUE16(parser->data[4], parser->data[5]);
    msg->type              = SDS011_MSG_TYPE_DEV_ID;
    msg->op                = SDS011_MSG_OP_SET;
    msg->src               = SDS011_MSG_SRC_SENSOR;
    return true;
  }
  return false;
}

static bool parse_sleep(sds011_parser_t const *parser, sds011_msg_t *msg) {
  uint8_t op = parser->data[1];
  uint8_t sl = parser->data[2];

  if (op != SDS011_MSG_OP_GET && op != SDS011_MSG_OP_SET) {
    return false;
  }
  if (sl != SDS011_SLEEP_ON && sl != SDS011_SLEEP_OFF) {
    return false;
  }
  if (parser->cmd == SDS011_CMD_QUERY) {
    msg->dev_id           = VALUE16(parser->data[13], parser->data[14]);
    msg->type             = SDS011_MSG_TYPE_SLEEP;
    msg->op               = op;
    msg->src              = SDS011_MSG_SRC_HOST;
    msg->data.sleep.value = sl;
    return true;
  }
  if (parser->cmd == SDS011_CMD_REPLY) {
    msg->dev_id               = VALUE16(parser->data[4], parser->data[5]);
    msg->type                 = SDS011_MSG_TYPE_SLEEP;
    msg->op                   = op;
    msg->src                  = SDS011_MSG_SRC_SENSOR;
    msg->data.sleep.value = sl;
    return true;
  }
  return false;
}

static bool parse_fw_ver(sds011_parser_t const *parser, sds011_msg_t *msg) {
  return true;
}

static bool parse_op_mode(sds011_parser_t const *parser, sds011_msg_t *msg) {
  return true;
}

void sds011_parser_clear(sds011_parser_t *parser) {
  parser->state     = 0;
  parser->data_len  = 0;
  parser->data_iter = 0;
  parser->data_crc  = 0;
}

sds011_parser_err_t sds011_parser_get_error(sds011_parser_t const *parser) {
  return parser->error;
}
