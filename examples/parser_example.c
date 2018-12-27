#include "../src/sds011_parser.h"

#include <stdint.h>
#include <stdio.h>

static uint32_t bytes_available(void);
static uint8_t read_byte(void);

int main(void) {
  sds011_parser_res_t parser_res;

  sds011_parser_t parser;
  sds011_parser_init(&parser);

  while (bytes_available() > 0) {
    parser_res = sds011_parser_parse(&parser, read_byte());

    if (parser_res == SDS011_PARSER_RES_READY) {
      printf("Completed\n");

      sds011_msg_t msg;
      sds011_parser_get_msg(&parser, &msg);

      if (msg.type == SDS011_MSG_TYPE_DATA) {
        printf("Device ID: %4X\n", msg.dev_id);
        printf("PM2.5:     %.1f ug/m3\n", msg.data.sample.pm2_5 / 10.0F);
        printf("PM10:      %.1f ug/m3\n", msg.data.sample.pm10 / 10.0F);
      }
    }
    if (parser_res == SDS011_PARSER_RES_ERROR) {
      printf("Error: %d\n", sds011_parser_get_error(&parser));
    }
  }

  return 0;
}

// UART Mock
static uint32_t _buffer_iter = 0;
static uint8_t _buffer[] = { 0xAA, 0xC0, 0xD4, 0x04, 0x3A, 0x0A, 0xA1, 0x60, 0x1D, 0xAB };

static uint32_t bytes_available(void) {
  return sizeof(_buffer) - _buffer_iter;
}

static uint8_t read_byte(void) {
  if (_buffer_iter < sizeof(_buffer)) {
    return _buffer[_buffer_iter++];
  }
  return 0;
}