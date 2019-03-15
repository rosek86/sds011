#include "sds011_parser.h"

#include <string.h>

#define VALUE16(hi, lo) (((uint16_t)(hi) << 8) | (uint16_t)(lo))

enum {
  STATE_BEG,
  STATE_CMD,
  STATE_DATA,
  STATE_CRC,
  STATE_END,
};

static void parser_clear(sds011_parser_t *parser);

void sds011_parser_init(sds011_parser_t *parser) {
  parser_clear(parser);
}

static void parser_clear(sds011_parser_t *parser) {
  parser->state     = 0;
  parser->cmd       = 0;
  parser->data_len  = 0;
  parser->data_iter = 0;
  parser->data_crc  = 0;
  parser->error     = SDS011_OK;
}

static uint8_t parser_data_len_by_cmd(uint8_t cmd);
static sds011_parser_res_t parser_error(sds011_parser_t *parser, uint32_t err_code);
static sds011_parser_res_t parser_completed(sds011_parser_t *parser);

sds011_parser_res_t sds011_parser_parse(sds011_parser_t *parser, uint8_t byte) {
  switch (parser->state) {
    case STATE_BEG:
      if (byte != SDS011_FRAME_BEG) {
        return parser_error(parser, SDS011_ERR_PARSER_FRAME_BEG);
      }
      parser->state++;
      break;
    case STATE_CMD:
      if ((parser->data_len = parser_data_len_by_cmd(byte)) == 0) {
        return parser_error(parser, SDS011_ERR_PARSER_CMD);
      }
      parser->cmd = byte;
      parser->data_iter = 0;
      parser->state++;
      break;
    case STATE_DATA:
      parser->data[parser->data_iter++] = byte;
      parser->data_crc += byte;

      if (parser->data_iter >= parser->data_len) {
        parser->state++;
      }
      break;
    case STATE_CRC:
      if (parser->data_crc != byte) {
        return parser_error(parser, SDS011_ERR_PARSER_CRC);
      }
      parser->state++;
      break;
    case STATE_END:
      if (byte != SDS011_FRAME_END) {
        return parser_error(parser, SDS011_ERR_PARSER_FRAME_END);
      }
      return parser_completed(parser);
    default:
      parser_clear(parser);
      break;
  }

  return SDS011_PARSER_RES_RUNNING;
}

static uint8_t parser_data_len_by_cmd(uint8_t cmd) {
  if (cmd == SDS011_CMD_QUERY) {
    return SDS011_QUERY_DATA_SIZE;
  }
  if (cmd == SDS011_CMD_REPLY || cmd == SDS011_DAT_REPLY) {
    return SDS011_REPLY_DATA_SIZE;
  }
  return 0;
}

static sds011_parser_res_t parser_error(sds011_parser_t *parser, uint32_t err_code) {
  parser_clear(parser);
  parser->error = err_code;
  return SDS011_PARSER_RES_ERROR;
}

static sds011_err_t get_msg(sds011_parser_t const *parser, sds011_msg_t *msg);

static sds011_parser_res_t parser_completed(sds011_parser_t *parser) {
  sds011_parser_res_t result = SDS011_PARSER_RES_READY;

  sds011_err_t err_code;
  if ((err_code = get_msg(parser, &parser->msg)) != SDS011_OK) {
    result = SDS011_PARSER_RES_ERROR;
  }

  parser_clear(parser);
  parser->error = err_code;

  return result;
}

static sds011_msg_type_t get_msg_type(sds011_parser_t const *parser);

// parsers
static sds011_err_t parse_rep_mode(sds011_parser_t const *parser, sds011_msg_t *msg);
static sds011_err_t parse_data(sds011_parser_t const *parser, sds011_msg_t *msg);
static sds011_err_t parse_dev_id(sds011_parser_t const *parser, sds011_msg_t *msg);
static sds011_err_t parse_sleep(sds011_parser_t const *parser, sds011_msg_t *msg);
static sds011_err_t parse_fw_ver(sds011_parser_t const *parser, sds011_msg_t *msg);
static sds011_err_t parse_op_mode(sds011_parser_t const *parser, sds011_msg_t *msg);

static sds011_err_t (*_msg_parsers[9])(sds011_parser_t const *parser, sds011_msg_t *msg) = {
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

static sds011_err_t get_msg(sds011_parser_t const *parser, sds011_msg_t *msg) {
  sds011_msg_type_t type = get_msg_type(parser);

  if (type >= SDS011_MSG_TYPE_COUNT) {
    return SDS011_ERR_INVALID_MSG_TYPE;
  }
  if (_msg_parsers[type] == NULL) {
    return SDS011_ERR_INVALID_MSG_TYPE;
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

static sds011_err_t parse_rep_mode(sds011_parser_t const *parser, sds011_msg_t *msg) {
  sds011_msg_op_t   op = (sds011_msg_op_t)  parser->data[1];
  sds011_rep_mode_t rm = (sds011_rep_mode_t)parser->data[2];

  if (op != SDS011_MSG_OP_GET && op != SDS011_MSG_OP_SET) {
    return SDS011_ERR_INVALID_DATA;
  }
  if (rm != SDS011_REP_MODE_ACTIVE && rm != SDS011_REP_MODE_QUERY) {
    return SDS011_ERR_INVALID_DATA;
  }
  if (parser->cmd == SDS011_CMD_QUERY) {
    msg->dev_id               = VALUE16(parser->data[13], parser->data[14]);
    msg->type                 = SDS011_MSG_TYPE_REP_MODE;
    msg->op                   = op;
    msg->src                  = SDS011_MSG_SRC_HOST;
    msg->data.rep_mode        = rm;
  } else {
    msg->dev_id               = VALUE16(parser->data[4], parser->data[5]);
    msg->type                 = SDS011_MSG_TYPE_REP_MODE;
    msg->op                   = op;
    msg->src                  = SDS011_MSG_SRC_SENSOR;
    msg->data.rep_mode        = rm;
  }
  return SDS011_OK;
}

static sds011_err_t parse_data(sds011_parser_t const *parser, sds011_msg_t *msg) {
  if (parser->cmd == SDS011_CMD_QUERY) {
    msg->dev_id            = VALUE16(parser->data[13], parser->data[14]);
    msg->type              = SDS011_MSG_TYPE_DATA;
    msg->op                = SDS011_MSG_OP_GET;
    msg->src               = SDS011_MSG_SRC_HOST;
  } else {
    msg->dev_id            = VALUE16(parser->data[4], parser->data[5]);
    msg->type              = SDS011_MSG_TYPE_DATA;
    msg->op                = SDS011_MSG_OP_GET;
    msg->src               = SDS011_MSG_SRC_SENSOR;
    msg->data.sample.pm2_5 = VALUE16(parser->data[1], parser->data[0]);
    msg->data.sample.pm10  = VALUE16(parser->data[3], parser->data[2]);
  }
  return SDS011_OK;
}

static sds011_err_t parse_dev_id(sds011_parser_t const *parser, sds011_msg_t *msg) {
  if (parser->cmd == SDS011_CMD_QUERY) {
    msg->dev_id          = VALUE16(parser->data[13], parser->data[14]);
    msg->type            = SDS011_MSG_TYPE_DEV_ID;
    msg->op              = SDS011_MSG_OP_SET;
    msg->src             = SDS011_MSG_SRC_HOST;
    msg->data.new_dev_id = VALUE16(parser->data[11], parser->data[12]);
  } else {
    msg->dev_id            = VALUE16(parser->data[4], parser->data[5]);
    msg->type              = SDS011_MSG_TYPE_DEV_ID;
    msg->op                = SDS011_MSG_OP_SET;
    msg->src               = SDS011_MSG_SRC_SENSOR;
  }
  return SDS011_OK;
}

static sds011_err_t parse_sleep(sds011_parser_t const *parser, sds011_msg_t *msg) {
  sds011_msg_op_t op = (sds011_msg_op_t)parser->data[1];
  sds011_sleep_t  sl = (sds011_sleep_t) parser->data[2];

  if (op != SDS011_MSG_OP_GET && op != SDS011_MSG_OP_SET) {
    return SDS011_ERR_INVALID_DATA;
  }
  if (sl != SDS011_SLEEP_ON && sl != SDS011_SLEEP_OFF) {
    return SDS011_ERR_INVALID_DATA;
  }
  if (parser->cmd == SDS011_CMD_QUERY) {
    msg->dev_id     = VALUE16(parser->data[13], parser->data[14]);
    msg->type       = SDS011_MSG_TYPE_SLEEP;
    msg->op         = op;
    msg->src        = SDS011_MSG_SRC_HOST;
    msg->data.sleep = sl;
  } else {
    msg->dev_id     = VALUE16(parser->data[4], parser->data[5]);
    msg->type       = SDS011_MSG_TYPE_SLEEP;
    msg->op         = op;
    msg->src        = SDS011_MSG_SRC_SENSOR;
    msg->data.sleep = sl;
  }
  return SDS011_OK;
}

static sds011_err_t parse_fw_ver(sds011_parser_t const *parser, sds011_msg_t *msg) {
  if (parser->cmd == SDS011_CMD_QUERY) {
    msg->dev_id             = VALUE16(parser->data[13], parser->data[14]);
    msg->type               = SDS011_MSG_TYPE_FW_VER;
    msg->op                 = SDS011_MSG_OP_GET;
    msg->src                = SDS011_MSG_SRC_HOST;
  } else {
    msg->dev_id             = VALUE16(parser->data[4], parser->data[5]);
    msg->type               = SDS011_MSG_TYPE_FW_VER;
    msg->op                 = SDS011_MSG_OP_GET;
    msg->src                = SDS011_MSG_SRC_SENSOR;
    msg->data.fw_ver.year   = parser->data[1];
    msg->data.fw_ver.month  = parser->data[2];
    msg->data.fw_ver.day    = parser->data[3];
  }
  return SDS011_OK;
}

static sds011_err_t parse_op_mode(sds011_parser_t const *parser, sds011_msg_t *msg) {
  const uint8_t max_interval = 30;

  sds011_msg_op_t op = (sds011_msg_op_t)parser->data[1];
  uint8_t interval = parser->data[2];
  uint8_t mode = SDS011_OP_MODE_CONTINOUS;

  if (op != SDS011_MSG_OP_GET && op != SDS011_MSG_OP_SET) {
    return SDS011_ERR_INVALID_DATA;
  }
  if (interval > max_interval) {
    return SDS011_ERR_INVALID_DATA;
  }
  if (interval != 0) {
    mode = SDS011_OP_MODE_INTERVAL;
  }

  if (parser->cmd == SDS011_CMD_QUERY) {
    msg->dev_id                 = VALUE16(parser->data[13], parser->data[14]);
    msg->type                   = SDS011_MSG_TYPE_OP_MODE;
    msg->op                     = op;
    msg->src                    = SDS011_MSG_SRC_HOST;
    msg->data.op_mode.mode      = mode;
    msg->data.op_mode.interval  = interval;
  } else {
    msg->dev_id                 = VALUE16(parser->data[4], parser->data[5]);
    msg->type                   = SDS011_MSG_TYPE_OP_MODE;
    msg->op                     = op;
    msg->src                    = SDS011_MSG_SRC_SENSOR;
    msg->data.op_mode.mode      = mode;
    msg->data.op_mode.interval  = interval;
  }
  return SDS011_OK;
}

void sds011_parser_get_msg(sds011_parser_t const *parser, sds011_msg_t *msg) {
  memcpy(msg, &parser->msg, sizeof(sds011_msg_t));
}

sds011_err_t sds011_parser_get_error(sds011_parser_t const *parser) {
  return parser->error;
}
