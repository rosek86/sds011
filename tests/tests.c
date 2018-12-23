#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "../src/sds011.h"

static sds011_parser_t parser;

/* A test case that does nothing and succeeds. */
static void test_parser_sync_byte(void **state) {
  (void) state; /* unused */
  sds011_parser_clear(&parser);

  // invalid
  assert_int_equal(sds011_parser_parse(&parser, 0x00), SDS011_PARSER_RES_ERROR);
  assert_int_equal(parser.state, 0);
  assert_int_equal(sds011_parser_get_error(&parser), SDS011_PARSER_ERR_FRAME_BEG);

  // okay
  assert_int_equal(sds011_parser_parse(&parser, 0xAA), SDS011_PARSER_RES_RUNNING);
  assert_int_equal(parser.state, 1);
}

static void test_parser_payload_len(void **state) {
  (void) state; /* unused */

  // SDS011_CMD_QUERY
  sds011_parser_clear(&parser);
  assert_int_equal(sds011_parser_parse(&parser, 0xAA), SDS011_PARSER_RES_RUNNING);
  assert_int_equal(sds011_parser_parse(&parser, 0xB4), SDS011_PARSER_RES_RUNNING);
  assert_int_equal(parser.state, 2);
  assert_int_equal(parser.data_len, 15);

  // SDS011_CMD_REPLY
  sds011_parser_clear(&parser);
  assert_int_equal(sds011_parser_parse(&parser, 0xAA), SDS011_PARSER_RES_RUNNING);
  assert_int_equal(sds011_parser_parse(&parser, 0xC5), SDS011_PARSER_RES_RUNNING);
  assert_int_equal(parser.state, 2);
  assert_int_equal(parser.data_len, 6);

  // SDS011_DAT_REPLY
  sds011_parser_clear(&parser);
  assert_int_equal(sds011_parser_parse(&parser, 0xAA), SDS011_PARSER_RES_RUNNING);
  assert_int_equal(sds011_parser_parse(&parser, 0xC0), SDS011_PARSER_RES_RUNNING);
  assert_int_equal(parser.state, 2);
  assert_int_equal(parser.data_len, 6);

  // Invalid command
  sds011_parser_clear(&parser);
  assert_int_equal(sds011_parser_parse(&parser, 0xAA), SDS011_PARSER_RES_RUNNING);
  assert_int_equal(sds011_parser_parse(&parser, 0x00), SDS011_PARSER_RES_ERROR);
  assert_int_equal(parser.state, 0);
  assert_int_equal(sds011_parser_get_error(&parser), SDS011_PARSER_ERR_CMD);
}

void test_parser_crc(void **state) {
  (void) state; /* unused */

  uint8_t msg[] = { 0xAA, 0xC0, 0xD4, 0x04, 0x3A, 0x0A, 0xA1, 0x60, 0x1D, 0xAB };

  // Invalid CRC
  sds011_parser_clear(&parser);
  assert_int_equal(sds011_parser_parse(&parser, msg[0]), SDS011_PARSER_RES_RUNNING);
  assert_int_equal(sds011_parser_parse(&parser, msg[1]), SDS011_PARSER_RES_RUNNING);

  for (int i = 0; i < 6; i++) {
    assert_int_equal(sds011_parser_parse(&parser, msg[2+i]), SDS011_PARSER_RES_RUNNING);
  }

  assert_int_equal(sds011_parser_parse(&parser, 0x1E), SDS011_PARSER_RES_ERROR);
  assert_int_equal(sds011_parser_get_error(&parser), SDS011_PARSER_ERR_CRC);

  // okay
  sds011_parser_clear(&parser);
  assert_int_equal(sds011_parser_parse(&parser, msg[0]), SDS011_PARSER_RES_RUNNING);
  assert_int_equal(sds011_parser_parse(&parser, msg[1]), SDS011_PARSER_RES_RUNNING);

  for (int i = 0; i < 6; i++) {
    assert_int_equal(sds011_parser_parse(&parser, msg[2+i]), SDS011_PARSER_RES_RUNNING);
  }

  assert_int_equal(sds011_parser_parse(&parser, msg[8]), SDS011_PARSER_RES_RUNNING);
}

void test_parser_end_frame(void **state) {
  (void) state; /* unused */

  uint8_t msg[] = { 0xAA, 0xC0, 0xD4, 0x04, 0x3A, 0x0A, 0xA1, 0x60, 0x1D, 0xAB };

  // Invalid end frame
  sds011_parser_clear(&parser);
  for (int i = 0; i < 9; i++) {
    assert_int_equal(sds011_parser_parse(&parser, msg[i]), SDS011_PARSER_RES_RUNNING);
  }
  assert_int_equal(sds011_parser_parse(&parser, 0xAC), SDS011_PARSER_RES_ERROR);
  assert_int_equal(sds011_parser_get_error(&parser), SDS011_PARSER_ERR_FRAME_END);

  // okay
  sds011_parser_clear(&parser);
  for (int i = 0; i < 9; i++) {
    assert_int_equal(sds011_parser_parse(&parser, msg[i]), SDS011_PARSER_RES_RUNNING);
  }
  assert_int_equal(sds011_parser_parse(&parser, msg[9]), SDS011_PARSER_RES_READY);
}


void test_parser_msg_data_value(void **state) {
  (void) state; /* unused */

  uint8_t buf[] = { 0xAA, 0xC0, 0xD4, 0x04, 0x3A, 0x0A, 0xA1, 0x60, 0x1D, 0xAB };

  sds011_parser_clear(&parser);
  for (int i = 0; i < 9; i++) {
    assert_int_equal(sds011_parser_parse(&parser, buf[i]), SDS011_PARSER_RES_RUNNING);
  }
  assert_int_equal(sds011_parser_parse(&parser, buf[9]), SDS011_PARSER_RES_READY);

  sds011_msg_t msg;
  assert_true(sds011_parser_get_msg(&parser, &msg));

  assert_int_equal(msg.type,                SDS011_MSG_TYPE_SAMPLE);
  assert_int_equal(msg.dev_id,              0xA160);
  assert_int_equal(msg.data.sample.pm2_5,   1236);
  assert_int_equal(msg.data.sample.pm10,    2618);
}

int main(void) {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_parser_sync_byte),
    cmocka_unit_test(test_parser_payload_len),
    cmocka_unit_test(test_parser_crc),
    cmocka_unit_test(test_parser_end_frame),
    cmocka_unit_test(test_parser_msg_data_value)
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
