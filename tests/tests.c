#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "../src/sds011.h"

static sds011_parser_t parser;

static void parse_buffer(uint8_t *buf, size_t size) {
  sds011_parser_clear(&parser);
  for (int i = 0; i < size - 1; i++) {
    assert_int_equal(sds011_parser_parse(&parser, buf[i]), SDS011_PARSER_RES_RUNNING);
  }
  assert_int_equal(sds011_parser_parse(&parser, buf[size - 1]), SDS011_PARSER_RES_READY);
}

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

void test_parser_msg_sample(void **state) {
  (void) state; /* unused */

  sds011_msg_t msg;

  // request
  uint8_t req[] = {
    0xAA, 0xB4, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0xFF, 0x02, 0xAB
  };
  parse_buffer(req, sizeof(req));

  assert_true(sds011_parser_get_msg(&parser, &msg));
  assert_int_equal(msg.dev_id,  0xFFFF);
  assert_int_equal(msg.type,    SDS011_MSG_TYPE_DATA);
  assert_int_equal(msg.op,      SDS011_MSG_OP_GET);
  assert_int_equal(msg.src,     SDS011_MSG_SRC_HOST);

  // response
  uint8_t res[] = {
    0xAA, 0xC0, 0xD4, 0x04, 0x3A, 0x0A, 0xA1, 0x60, 0x1D, 0xAB
  };
  parse_buffer(res, sizeof(res));

  assert_true(sds011_parser_get_msg(&parser, &msg));
  assert_int_equal(msg.dev_id,            0xA160);
  assert_int_equal(msg.type,              SDS011_MSG_TYPE_DATA);
  assert_int_equal(msg.op,                SDS011_MSG_OP_GET);
  assert_int_equal(msg.src,               SDS011_MSG_SRC_SENSOR);
  assert_int_equal(msg.data.sample.pm2_5, 1236);
  assert_int_equal(msg.data.sample.pm10,  2618);
}

void test_parser_msg_rep_mode_get(void **state) {
  (void) state; /* unused */

  sds011_msg_t msg;

  uint8_t req[] = {
    0xAA, 0xB4, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0xFF, 0x00, 0xAB
  };
  parse_buffer(req, sizeof(req));

  assert_true(sds011_parser_get_msg(&parser, &msg));
  assert_int_equal(msg.dev_id,  0xFFFF);
  assert_int_equal(msg.type,    SDS011_MSG_TYPE_REP_MODE);
  assert_int_equal(msg.op,      SDS011_MSG_OP_GET);
  assert_int_equal(msg.src,     SDS011_MSG_SRC_HOST);

  uint8_t res1[] = {
    0xAA, 0xC5, 0x02, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x03, 0xAB
  };
  parse_buffer(res1, sizeof(res1));

  assert_true(sds011_parser_get_msg(&parser, &msg));
  assert_int_equal(msg.dev_id,        0xA160);
  assert_int_equal(msg.type,          SDS011_MSG_TYPE_REP_MODE);
  assert_int_equal(msg.op,            SDS011_MSG_OP_GET);
  assert_int_equal(msg.src,           SDS011_MSG_SRC_SENSOR);
  assert_int_equal(msg.data.rep_mode, SDS011_MSG_REP_ACTIVE);

  uint8_t res2[] = {
    0xAA, 0xC5, 0x02, 0x00, 0x01, 0x00, 0xA1, 0x60, 0x04, 0xAB
  };
  parse_buffer(res2, sizeof(res2));

  assert_true(sds011_parser_get_msg(&parser, &msg));
  assert_int_equal(msg.dev_id,        0xA160);
  assert_int_equal(msg.type,          SDS011_MSG_TYPE_REP_MODE);
  assert_int_equal(msg.op,            SDS011_MSG_OP_GET);
  assert_int_equal(msg.src,           SDS011_MSG_SRC_SENSOR);
  assert_int_equal(msg.data.rep_mode, SDS011_MSG_REP_QUERY);

  // 0xAA 0xB4 0x02 0x01 0x01 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0xA1 0x60 0x05 0xAB
  // 0xAA 0xC5 0x02 0x01 0x01 0x00 0xA1 0x60 0x05 0xAB
}

int main(void) {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_parser_sync_byte),
    cmocka_unit_test(test_parser_payload_len),
    cmocka_unit_test(test_parser_crc),
    cmocka_unit_test(test_parser_end_frame),
    cmocka_unit_test(test_parser_msg_sample),
    cmocka_unit_test(test_parser_msg_rep_mode_get),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
