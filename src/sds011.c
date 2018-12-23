#include "sds011.h"

#define SDS011_SYNC 0xAA

#define SDS011_CMD_QUERY 0xB4
#define SDS011_CMD_REPLY 0xC5
#define SDS011_DAT_REPLY 0xC0

static uint8_t payload_len_by_cmd(uint8_t cmd);

bool sds011_parser_parse(sds011_parser_t *parser, uint8_t byte) {
  switch (parser->state) {
    case 0:
      if (byte == SDS011_SYNC) {
        parser->state++;
      }
      break;
    case 1:
      if ((parser->payload_len = payload_len_by_cmd(byte)) > 0) {
        parser->state++;
      } else {
        parser->state = 0;
      }
      break;
    case 2:
      // payload
      break;
    case 3:
      // checksum
      break;
    case 4:
      // end frame
      return true;
      break;
  }

  return false;
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
  parser->payload_len = 0;
}
