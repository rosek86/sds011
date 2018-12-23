#include "sds011.h"

#include <string.h>

#define SDS011_FRAME_BEG 0xAA
#define SDS011_FRAME_END 0xAB

#define SDS011_CMD_QUERY 0xB4
#define SDS011_CMD_REPLY 0xC5
#define SDS011_DAT_REPLY 0xC0

#define VALUE16(hi, lo) (((uint16_t)(hi) << 8) | (lo))

typedef enum {
  STATE_BEG  = 0,
  STATE_CMD  = 1,
  STATE_DATA = 2,
  STATE_CRC  = 3,
  STATE_END  = 4
} state_t;

static uint8_t payload_len_by_cmd(uint8_t cmd);

sds011_parser_ret_t sds011_parser_parse(sds011_parser_t *parser, uint8_t byte) {
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
    return 15;
  }
  if (cmd == SDS011_CMD_REPLY || cmd == SDS011_DAT_REPLY) {
    return 6;
  }
  return 0;
}

static sds011_msg_type_t get_msg_type(sds011_parser_t const *parser);
static void get_sample(sds011_parser_t const *parser, sds011_msg_t *msg);

bool sds011_parser_get_msg(sds011_parser_t const *parser, sds011_msg_t *msg) {
  memset(msg, 0, sizeof(sds011_msg_t));

  msg->type = get_msg_type(parser);

  if (msg->type == SDS011_MSG_TYPE_SAMPLE) {
    get_sample(parser, msg);
    return true;
  }

  return false;
}

static sds011_msg_type_t get_msg_type(sds011_parser_t const *parser) {
  if (parser->cmd == SDS011_DAT_REPLY) {
    return SDS011_MSG_TYPE_SAMPLE;
  }

  return parser->data[0];
}


static void get_sample(sds011_parser_t const *parser, sds011_msg_t *msg) {
  msg->dev_id            = VALUE16(parser->data[4], parser->data[5]);
  msg->op                = SDS011_MSG_OP_GET;
  msg->data.sample.pm2_5 = VALUE16(parser->data[1], parser->data[0]);
  msg->data.sample.pm10  = VALUE16(parser->data[3], parser->data[2]);
}

void sds011_parser_clear(sds011_parser_t *parser) {
  parser->state = 0;
  parser->data_len = 0;
  parser->data_iter = 0;
  parser->data_crc = 0;
}

sds011_parser_err_t sds011_parser_get_error(sds011_parser_t const *parser) {
  return parser->error;
}
