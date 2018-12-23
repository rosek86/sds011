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
  assert_int_equal(parser.error, SDS011_PARSER_ERR_FRAME_BEG);

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
  assert_int_equal(parser.error, SDS011_PARSER_ERR_CMD);
}

int main(void) {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_parser_sync_byte),
    cmocka_unit_test(test_parser_payload_len),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
