#include "sds011.h"

#define SDS011_FRAME_BEG 0xAA
#define SDS011_FRAME_END 0xAB

#define SDS011_CMD_QUERY 0xB4
#define SDS011_CMD_REPLY 0xC5
#define SDS011_DAT_REPLY 0xC0

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

void sds011_parser_clear(sds011_parser_t *parser) {
  parser->state = 0;
  parser->data_len = 0;
  parser->data_iter = 0;
  parser->data_crc = 0;
}
