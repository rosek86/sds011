#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>

#include "../src/sds011_parser.h"

static sds011_parser_t parser;

static void parse_buffer(uint8_t *buf, size_t size, sds011_msg_t *msg) {
  for (size_t i = 0; i < size - 1; i++) {
    sds011_parser_res_t res = sds011_parser_parse(&parser, buf[i]);
    assert_int_equal(res, SDS011_PARSER_RES_RUNNING);
  }
  assert_int_equal(sds011_parser_parse(&parser, buf[size - 1]), SDS011_PARSER_RES_READY);
  sds011_parser_get_msg(&parser, msg);
}

/* A test case that does nothing and succeeds. */
static void test_parser_sync_byte(void **state) {
  (void) state; /* unused */
  sds011_parser_init(&parser);

  // invalid
  assert_int_equal(sds011_parser_parse(&parser, 0x00), SDS011_PARSER_RES_ERROR);
  assert_int_equal(parser.state, 0);
  assert_int_equal(sds011_parser_get_error(&parser), SDS011_ERR_PARSER_FRAME_BEG);

  // okay
  assert_int_equal(sds011_parser_parse(&parser, 0xAA), SDS011_PARSER_RES_RUNNING);
  assert_int_equal(parser.state, 1);
}

static void test_parser_payload_len(void **state) {
  (void) state; /* unused */

  // SDS011_CMD_QUERY
  sds011_parser_init(&parser);
  assert_int_equal(sds011_parser_parse(&parser, 0xAA), SDS011_PARSER_RES_RUNNING);
  assert_int_equal(sds011_parser_parse(&parser, 0xB4), SDS011_PARSER_RES_RUNNING);
  assert_int_equal(parser.state, 2);
  assert_int_equal(parser.data_len, 15);

  // SDS011_CMD_REPLY
  sds011_parser_init(&parser);
  assert_int_equal(sds011_parser_parse(&parser, 0xAA), SDS011_PARSER_RES_RUNNING);
  assert_int_equal(sds011_parser_parse(&parser, 0xC5), SDS011_PARSER_RES_RUNNING);
  assert_int_equal(parser.state, 2);
  assert_int_equal(parser.data_len, 6);

  // SDS011_DAT_REPLY
  sds011_parser_init(&parser);
  assert_int_equal(sds011_parser_parse(&parser, 0xAA), SDS011_PARSER_RES_RUNNING);
  assert_int_equal(sds011_parser_parse(&parser, 0xC0), SDS011_PARSER_RES_RUNNING);
  assert_int_equal(parser.state, 2);
  assert_int_equal(parser.data_len, 6);

  // Invalid command
  sds011_parser_init(&parser);
  assert_int_equal(sds011_parser_parse(&parser, 0xAA), SDS011_PARSER_RES_RUNNING);
  assert_int_equal(sds011_parser_parse(&parser, 0x00), SDS011_PARSER_RES_ERROR);
  assert_int_equal(parser.state, 0);
  assert_int_equal(sds011_parser_get_error(&parser), SDS011_ERR_PARSER_CMD);
}

void test_parser_crc(void **state) {
  (void) state; /* unused */
  sds011_parser_init(&parser);

  uint8_t msg[] = { 0xAA, 0xC0, 0xD4, 0x04, 0x3A, 0x0A, 0xA1, 0x60, 0x1D, 0xAB };

  // Invalid CRC
  assert_int_equal(sds011_parser_parse(&parser, msg[0]), SDS011_PARSER_RES_RUNNING);
  assert_int_equal(sds011_parser_parse(&parser, msg[1]), SDS011_PARSER_RES_RUNNING);

  for (int i = 0; i < 6; i++) {
    assert_int_equal(sds011_parser_parse(&parser, msg[2+i]), SDS011_PARSER_RES_RUNNING);
  }

  assert_int_equal(sds011_parser_parse(&parser, 0x1E), SDS011_PARSER_RES_ERROR);
  assert_int_equal(sds011_parser_get_error(&parser), SDS011_ERR_PARSER_CRC);

  // okay
  assert_int_equal(sds011_parser_parse(&parser, msg[0]), SDS011_PARSER_RES_RUNNING);
  assert_int_equal(sds011_parser_parse(&parser, msg[1]), SDS011_PARSER_RES_RUNNING);

  for (int i = 0; i < 6; i++) {
    assert_int_equal(sds011_parser_parse(&parser, msg[2+i]), SDS011_PARSER_RES_RUNNING);
  }

  assert_int_equal(sds011_parser_parse(&parser, msg[8]), SDS011_PARSER_RES_RUNNING);
}

void test_parser_end_frame(void **state) {
  (void) state; /* unused */
  sds011_parser_init(&parser);

  uint8_t msg[] = { 0xAA, 0xC0, 0xD4, 0x04, 0x3A, 0x0A, 0xA1, 0x60, 0x1D, 0xAB };

  // Invalid end frame
  for (int i = 0; i < 9; i++) {
    assert_int_equal(sds011_parser_parse(&parser, msg[i]), SDS011_PARSER_RES_RUNNING);
  }
  assert_int_equal(sds011_parser_parse(&parser, 0xAC), SDS011_PARSER_RES_ERROR);
  assert_int_equal(sds011_parser_get_error(&parser), SDS011_ERR_PARSER_FRAME_END);

  // okay
  for (int i = 0; i < 9; i++) {
    assert_int_equal(sds011_parser_parse(&parser, msg[i]), SDS011_PARSER_RES_RUNNING);
  }
  assert_int_equal(sds011_parser_parse(&parser, msg[9]), SDS011_PARSER_RES_READY);
}

void test_parser_msg_rep_mode_get(void **state) {
  (void) state; /* unused */
  sds011_parser_init(&parser);

  sds011_msg_t msg;

  uint8_t req[] = {
    0xAA, 0xB4, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0xFF, 0x00, 0xAB
  };
  parse_buffer(req, sizeof(req), &msg);

  assert_int_equal(msg.dev_id,  0xFFFF);
  assert_int_equal(msg.type,    SDS011_MSG_TYPE_REP_MODE);
  assert_int_equal(msg.op,      SDS011_MSG_OP_GET);
  assert_int_equal(msg.src,     SDS011_MSG_SRC_HOST);

  uint8_t res1[] = {
    0xAA, 0xC5, 0x02, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x03, 0xAB
  };
  parse_buffer(res1, sizeof(res1), &msg);

  assert_int_equal(msg.dev_id,        0xA160);
  assert_int_equal(msg.type,          SDS011_MSG_TYPE_REP_MODE);
  assert_int_equal(msg.op,            SDS011_MSG_OP_GET);
  assert_int_equal(msg.src,           SDS011_MSG_SRC_SENSOR);
  assert_int_equal(msg.data.rep_mode, SDS011_REP_MODE_ACTIVE);

  uint8_t res2[] = {
    0xAA, 0xC5, 0x02, 0x00, 0x01, 0x00, 0xA1, 0x60, 0x04, 0xAB
  };
  parse_buffer(res2, sizeof(res2), &msg);

  assert_int_equal(msg.dev_id,        0xA160);
  assert_int_equal(msg.type,          SDS011_MSG_TYPE_REP_MODE);
  assert_int_equal(msg.op,            SDS011_MSG_OP_GET);
  assert_int_equal(msg.src,           SDS011_MSG_SRC_SENSOR);
  assert_int_equal(msg.data.rep_mode, SDS011_REP_MODE_QUERY);
}

void test_parser_msg_rep_mode_set(void **state) {
  (void) state; /* unused */
  sds011_parser_init(&parser);

  sds011_msg_t msg;

  uint8_t req[] = {
    0xAA, 0xB4, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1,
    0x60, 0x05, 0xAB
  };
  parse_buffer(req, sizeof(req), &msg);

  assert_int_equal(msg.dev_id,        0xA160);
  assert_int_equal(msg.type,          SDS011_MSG_TYPE_REP_MODE);
  assert_int_equal(msg.op,            SDS011_MSG_OP_SET);
  assert_int_equal(msg.src,           SDS011_MSG_SRC_HOST);
  assert_int_equal(msg.data.rep_mode, SDS011_REP_MODE_QUERY);

  uint8_t res[] = {
    0xAA, 0xC5, 0x02, 0x01, 0x01, 0x00, 0xA1, 0x60, 0x05, 0xAB
  };
  parse_buffer(res, sizeof(res), &msg);

  assert_int_equal(msg.dev_id,        0xA160);
  assert_int_equal(msg.type,          SDS011_MSG_TYPE_REP_MODE);
  assert_int_equal(msg.op,            SDS011_MSG_OP_SET);
  assert_int_equal(msg.src,           SDS011_MSG_SRC_SENSOR);
  assert_int_equal(msg.data.rep_mode, SDS011_REP_MODE_QUERY);
}

void test_parser_msg_sample(void **state) {
  (void) state; /* unused */
  sds011_parser_init(&parser);

  sds011_msg_t msg;

  // request
  uint8_t req[] = {
    0xAA, 0xB4, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0xFF, 0x02, 0xAB
  };
  parse_buffer(req, sizeof(req), &msg);

  assert_int_equal(msg.dev_id,  0xFFFF);
  assert_int_equal(msg.type,    SDS011_MSG_TYPE_DATA);
  assert_int_equal(msg.op,      SDS011_MSG_OP_GET);
  assert_int_equal(msg.src,     SDS011_MSG_SRC_HOST);

  // response
  uint8_t res[] = {
    0xAA, 0xC0, 0xD4, 0x04, 0x3A, 0x0A, 0xA1, 0x60, 0x1D, 0xAB
  };
  parse_buffer(res, sizeof(res), &msg);

  assert_int_equal(msg.dev_id,            0xA160);
  assert_int_equal(msg.type,              SDS011_MSG_TYPE_DATA);
  assert_int_equal(msg.op,                SDS011_MSG_OP_GET);
  assert_int_equal(msg.src,               SDS011_MSG_SRC_SENSOR);
  assert_int_equal(msg.data.sample.pm2_5, 1236);
  assert_int_equal(msg.data.sample.pm10,  2618);
}

void test_parser_msg_dev_id_set(void **state) {
  (void) state; /* unused */
  sds011_parser_init(&parser);

  sds011_msg_t msg;

  // request
  uint8_t req[] = {
    0xAA, 0xB4, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x01, 0xA1,
    0x60, 0xA7, 0xAB
  };
  parse_buffer(req, sizeof(req), &msg);

  assert_int_equal(msg.dev_id,          0xA160);
  assert_int_equal(msg.type,            SDS011_MSG_TYPE_DEV_ID);
  assert_int_equal(msg.op,              SDS011_MSG_OP_SET);
  assert_int_equal(msg.src,             SDS011_MSG_SRC_HOST);
  assert_int_equal(msg.data.new_dev_id, 0xA001);

  // response
  uint8_t res[] = {
    0xAA, 0xC5, 0x05, 0x00, 0x00, 0x00, 0xA0, 0x01, 0xA6, 0xAB
  };
  parse_buffer(res, sizeof(res), &msg);

  assert_int_equal(msg.dev_id, 0xA001);
  assert_int_equal(msg.type,   SDS011_MSG_TYPE_DEV_ID);
  assert_int_equal(msg.op,     SDS011_MSG_OP_SET);
  assert_int_equal(msg.src,    SDS011_MSG_SRC_SENSOR);
}

void test_parser_msg_sleep_set_on(void **state) {
  (void) state; /* unused */
  sds011_parser_init(&parser);

  sds011_msg_t msg;

  // request
  uint8_t req[] = {
    0xAA, 0xB4, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x08, 0xAB
  };
  parse_buffer(req, sizeof(req), &msg);

  assert_int_equal(msg.dev_id,     0xA160);
  assert_int_equal(msg.type,       SDS011_MSG_TYPE_SLEEP);
  assert_int_equal(msg.op,         SDS011_MSG_OP_SET);
  assert_int_equal(msg.src,        SDS011_MSG_SRC_HOST);
  assert_int_equal(msg.data.sleep, SDS011_SLEEP_ON);

  // response
  uint8_t res[] = {
    0xAA, 0xC5, 0x06, 0x01, 0x00, 0x00, 0xA1, 0x60, 0x08, 0xAB
  };
  parse_buffer(res, sizeof(res), &msg);
 
  assert_int_equal(msg.dev_id,     0xA160);
  assert_int_equal(msg.type,       SDS011_MSG_TYPE_SLEEP);
  assert_int_equal(msg.op,         SDS011_MSG_OP_SET);
  assert_int_equal(msg.src,        SDS011_MSG_SRC_SENSOR);
  assert_int_equal(msg.data.sleep, SDS011_SLEEP_ON);
}

void test_parser_msg_sleep_set_off(void **state) {
  (void) state; /* unused */
  sds011_parser_init(&parser);

  sds011_msg_t msg;

  // request
  uint8_t req[] = {
    0xAA, 0xB4, 0x06, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x09, 0xAB
  };
  parse_buffer(req, sizeof(req), &msg);

  assert_int_equal(msg.dev_id,     0xA160);
  assert_int_equal(msg.type,       SDS011_MSG_TYPE_SLEEP);
  assert_int_equal(msg.op,         SDS011_MSG_OP_SET);
  assert_int_equal(msg.src,        SDS011_MSG_SRC_HOST);
  assert_int_equal(msg.data.sleep, SDS011_SLEEP_OFF);

  // response
  uint8_t res[] = {
    0xAA, 0xC5, 0x06, 0x01, 0x01, 0x00, 0xA1, 0x60, 0x09, 0xAB
  };
  parse_buffer(res, sizeof(res), &msg);
 
  assert_int_equal(msg.dev_id,     0xA160);
  assert_int_equal(msg.type,       SDS011_MSG_TYPE_SLEEP);
  assert_int_equal(msg.op,         SDS011_MSG_OP_SET);
  assert_int_equal(msg.src,        SDS011_MSG_SRC_SENSOR);
  assert_int_equal(msg.data.sleep, SDS011_SLEEP_OFF);
}

void test_parser_msg_sleep_get(void **state) {
  (void) state; /* unused */
  sds011_parser_init(&parser);

  sds011_msg_t msg;

  // request
  uint8_t req[] = {
    0xAA, 0xB4, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x07, 0xAB
  };
  parse_buffer(req, sizeof(req), &msg);

  assert_int_equal(msg.dev_id, 0xA160);
  assert_int_equal(msg.type,   SDS011_MSG_TYPE_SLEEP);
  assert_int_equal(msg.op,     SDS011_MSG_OP_GET);
  assert_int_equal(msg.src,    SDS011_MSG_SRC_HOST);

  // response
  uint8_t res1[] = { 0xAA, 0xC5, 0x06, 0x00, 0x01, 0x00, 0xA1, 0x60, 0x08, 0xAB };
  parse_buffer(res1, sizeof(res1), &msg);
 
  assert_int_equal(msg.dev_id,     0xA160);
  assert_int_equal(msg.type,       SDS011_MSG_TYPE_SLEEP);
  assert_int_equal(msg.op,         SDS011_MSG_OP_GET);
  assert_int_equal(msg.src,        SDS011_MSG_SRC_SENSOR);
  assert_int_equal(msg.data.sleep, SDS011_SLEEP_OFF);

  uint8_t res2[] = { 0xAA, 0xC5, 0x06, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x07, 0xAB };
  parse_buffer(res2, sizeof(res2), &msg);
 
  assert_int_equal(msg.dev_id,     0xA160);
  assert_int_equal(msg.type,       SDS011_MSG_TYPE_SLEEP);
  assert_int_equal(msg.op,         SDS011_MSG_OP_GET);
  assert_int_equal(msg.src,        SDS011_MSG_SRC_SENSOR);
  assert_int_equal(msg.data.sleep, SDS011_SLEEP_ON);
}

void test_parser_msg_fw_ver_get(void **state) {
  (void) state; /* unused */
  sds011_parser_init(&parser);

  sds011_msg_t msg;

  // request
  uint8_t req[] = {
    0xAA, 0xB4, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x08, 0xAB
  };
  parse_buffer(req, sizeof(req), &msg);

  assert_int_equal(msg.dev_id, 0xA160);
  assert_int_equal(msg.type,   SDS011_MSG_TYPE_FW_VER);
  assert_int_equal(msg.op,     SDS011_MSG_OP_GET);
  assert_int_equal(msg.src,    SDS011_MSG_SRC_HOST);

  // response
  uint8_t res[] = {
    0xAA, 0xC5, 0x07, 0x0F, 0x07, 0x0A, 0xA1, 0x60, 0x28, 0xAB
  };
  parse_buffer(res, sizeof(res), &msg);

  assert_int_equal(msg.dev_id,            0xA160);
  assert_int_equal(msg.type,              SDS011_MSG_TYPE_FW_VER);
  assert_int_equal(msg.op,                SDS011_MSG_OP_GET);
  assert_int_equal(msg.src,               SDS011_MSG_SRC_SENSOR);
  assert_int_equal(msg.data.fw_ver.year,  15);
  assert_int_equal(msg.data.fw_ver.month, 7);
  assert_int_equal(msg.data.fw_ver.day,   10);
}

void test_parser_msg_op_mode_set_periodic(void **state) {
  (void) state; /* unused */
  sds011_parser_init(&parser);

  sds011_msg_t msg;

  // request
  uint8_t req[] = {
    0xAA, 0xB4, 0x08, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x0B, 0xAB
  };
  parse_buffer(req, sizeof(req), &msg);

  assert_int_equal(msg.dev_id,                0xA160);
  assert_int_equal(msg.type,                  SDS011_MSG_TYPE_OP_MODE);
  assert_int_equal(msg.op,                    SDS011_MSG_OP_SET);
  assert_int_equal(msg.src,                   SDS011_MSG_SRC_HOST);
  assert_int_equal(msg.data.op_mode.mode,     SDS011_OP_MODE_INTERVAL);
  assert_int_equal(msg.data.op_mode.interval, 1);

  // response
  uint8_t res[] = {
    0xAA, 0xC5, 0x08, 0x01, 0x01, 0x00, 0xA1, 0x60, 0x0B, 0xAB
  };
  parse_buffer(res, sizeof(res), &msg);

  assert_int_equal(msg.dev_id,                0xA160);
  assert_int_equal(msg.type,                  SDS011_MSG_TYPE_OP_MODE);
  assert_int_equal(msg.op,                    SDS011_MSG_OP_SET);
  assert_int_equal(msg.src,                   SDS011_MSG_SRC_SENSOR);
  assert_int_equal(msg.data.op_mode.mode,     SDS011_OP_MODE_INTERVAL);
  assert_int_equal(msg.data.op_mode.interval, 1);
}

void test_parser_msg_op_mode_set_continous(void **state) {
  (void) state; /* unused */
  sds011_parser_init(&parser);

  sds011_msg_t msg;

  // request
  uint8_t req[] = {
    0xAA, 0xB4, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x0A, 0xAB
  };
  parse_buffer(req, sizeof(req), &msg);

  assert_int_equal(msg.dev_id,                0xA160);
  assert_int_equal(msg.type,                  SDS011_MSG_TYPE_OP_MODE);
  assert_int_equal(msg.op,                    SDS011_MSG_OP_SET);
  assert_int_equal(msg.src,                   SDS011_MSG_SRC_HOST);
  assert_int_equal(msg.data.op_mode.mode,     SDS011_OP_MODE_CONTINOUS);
  assert_int_equal(msg.data.op_mode.interval, 0);

  // response
  uint8_t res[] = {
    0xAA, 0xC5, 0x08, 0x01, 0x00, 0x00, 0xA1, 0x60, 0x0A, 0xAB
  };
  parse_buffer(res, sizeof(res), &msg);

  assert_int_equal(msg.dev_id,                0xA160);
  assert_int_equal(msg.type,                  SDS011_MSG_TYPE_OP_MODE);
  assert_int_equal(msg.op,                    SDS011_MSG_OP_SET);
  assert_int_equal(msg.src,                   SDS011_MSG_SRC_SENSOR);
  assert_int_equal(msg.data.op_mode.mode,     SDS011_OP_MODE_CONTINOUS);
  assert_int_equal(msg.data.op_mode.interval, 0);
}

void test_parser_msg_op_mode_get(void **state) {
  (void) state; /* unused */
  sds011_parser_init(&parser);

  sds011_msg_t msg;

  // request
  uint8_t req[] = {
    0xAA, 0xB4, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x09, 0xAB
  };
  parse_buffer(req, sizeof(req), &msg);

  assert_int_equal(msg.dev_id,                0xA160);
  assert_int_equal(msg.type,                  SDS011_MSG_TYPE_OP_MODE);
  assert_int_equal(msg.op,                    SDS011_MSG_OP_GET);
  assert_int_equal(msg.src,                   SDS011_MSG_SRC_HOST);

  // response
  uint8_t res[] = {
    0xAA, 0xC5, 0x08, 0x00, 0x02, 0x00, 0xA1, 0x60, 0x0B, 0xAB
  };
  parse_buffer(res, sizeof(res), &msg);

  assert_int_equal(msg.dev_id,                0xA160);
  assert_int_equal(msg.type,                  SDS011_MSG_TYPE_OP_MODE);
  assert_int_equal(msg.op,                    SDS011_MSG_OP_GET);
  assert_int_equal(msg.src,                   SDS011_MSG_SRC_SENSOR);
  assert_int_equal(msg.data.op_mode.mode,     SDS011_OP_MODE_INTERVAL);
  assert_int_equal(msg.data.op_mode.interval, 2);
}

static void test_parser_invalid_state(void **state) {
  (void)state; // unused

  sds011_parser_t parser;
  parser.state = 16; // > STATE_END
  assert_int_equal(sds011_parser_parse(&parser, 'a'), SDS011_PARSER_RES_RUNNING);
  assert_int_equal(parser.state, 0);
}

static void test_parser_invalid_msg(void **state) {
  (void)state; // unused
  sds011_parser_init(&parser);

  // start, cmd, data..., crc, end

  uint8_t buf[] = { 0xAA, 0xC5, 0x09, 0x00, 0x02, 0x00, 0xA1, 0x60, 0x0C, 0xAB };
  for (size_t i = 0; i < sizeof(buf) - 1; i++) {
    sds011_parser_res_t res = sds011_parser_parse(&parser, buf[i]);
    assert_int_equal(res, SDS011_PARSER_RES_RUNNING);
  }
  assert_int_equal(sds011_parser_parse(&parser, buf[sizeof(buf) - 1]), SDS011_PARSER_RES_ERROR);
  assert_int_equal(sds011_parser_get_error(&parser), SDS011_ERR_INVALID_MSG_TYPE);

  buf[2] = 1;
  buf[8] = 0x0C - 0x08;
  for (size_t i = 0; i < sizeof(buf) - 1; i++) {
    sds011_parser_res_t res = sds011_parser_parse(&parser, buf[i]);
    assert_int_equal(res, SDS011_PARSER_RES_RUNNING);
  }
  assert_int_equal(sds011_parser_parse(&parser, buf[sizeof(buf) - 1]), SDS011_PARSER_RES_ERROR);
  assert_int_equal(sds011_parser_get_error(&parser), SDS011_ERR_INVALID_MSG_TYPE);
}

static void test_parser_rep_mode_invalid_data(void **state) {
  (void)state;

  uint8_t buf[] = { 0xAA, 0xC5, 0x02, 0x02, 0x00, 0x00, 0xA1, 0x60, 0x05, 0xAB };

  // invalid op
  for (size_t i = 0; i < sizeof(buf) - 1; i++) {
    sds011_parser_res_t res = sds011_parser_parse(&parser, buf[i]);
    assert_int_equal(res, SDS011_PARSER_RES_RUNNING);
  }
  assert_int_equal(sds011_parser_parse(&parser, buf[sizeof(buf) - 1]), SDS011_PARSER_RES_ERROR);
  assert_int_equal(sds011_parser_get_error(&parser), SDS011_ERR_INVALID_DATA);

  // invalid rep mode
  buf[3] = 0;
  buf[4] = 2;
  for (size_t i = 0; i < sizeof(buf) - 1; i++) {
    sds011_parser_res_t res = sds011_parser_parse(&parser, buf[i]);
    assert_int_equal(res, SDS011_PARSER_RES_RUNNING);
  }
  assert_int_equal(sds011_parser_parse(&parser, buf[sizeof(buf) - 1]), SDS011_PARSER_RES_ERROR);
  assert_int_equal(sds011_parser_get_error(&parser), SDS011_ERR_INVALID_DATA);
}

static void test_parser_sleep_invalid_data(void **state) {
  (void)state;

  uint8_t buf[] = { 0xAA, 0xC5, 0x06, 0x02, 0x00, 0x00, 0xA1, 0x60, 0x09, 0xAB };

  // invalid op
  for (size_t i = 0; i < sizeof(buf) - 1; i++) {
    sds011_parser_res_t res = sds011_parser_parse(&parser, buf[i]);
    assert_int_equal(res, SDS011_PARSER_RES_RUNNING);
  }
  assert_int_equal(sds011_parser_parse(&parser, buf[sizeof(buf) - 1]), SDS011_PARSER_RES_ERROR);
  assert_int_equal(sds011_parser_get_error(&parser), SDS011_ERR_INVALID_DATA);

  // invalid rep mode
  buf[3] = 0;
  buf[4] = 2;
  for (size_t i = 0; i < sizeof(buf) - 1; i++) {
    sds011_parser_res_t res = sds011_parser_parse(&parser, buf[i]);
    assert_int_equal(res, SDS011_PARSER_RES_RUNNING);
  }
  assert_int_equal(sds011_parser_parse(&parser, buf[sizeof(buf) - 1]), SDS011_PARSER_RES_ERROR);
  assert_int_equal(sds011_parser_get_error(&parser), SDS011_ERR_INVALID_DATA);
}

static void test_parser_op_mode_invalid_data(void **state) {
  (void)state;

  uint8_t buf[] = { 0xAA, 0xC5, 0x08, 0x02, 0x00, 0x00, 0xA1, 0x60, 0x0B, 0xAB };

  // invalid op
  for (size_t i = 0; i < sizeof(buf) - 1; i++) {
    sds011_parser_res_t res = sds011_parser_parse(&parser, buf[i]);
    assert_int_equal(res, SDS011_PARSER_RES_RUNNING);
  }
  assert_int_equal(sds011_parser_parse(&parser, buf[sizeof(buf) - 1]), SDS011_PARSER_RES_ERROR);
  assert_int_equal(sds011_parser_get_error(&parser), SDS011_ERR_INVALID_DATA);

  // invalid rep mode
  buf[3] = 0;
  buf[4] = 31;
  buf[8] = 0x28; // crc
  for (size_t i = 0; i < sizeof(buf) - 1; i++) {
    sds011_parser_res_t res = sds011_parser_parse(&parser, buf[i]);
    assert_int_equal(res, SDS011_PARSER_RES_RUNNING);
  }
  assert_int_equal(sds011_parser_parse(&parser, buf[sizeof(buf) - 1]), SDS011_PARSER_RES_ERROR);
  assert_int_equal(sds011_parser_get_error(&parser), SDS011_ERR_INVALID_DATA);
}

int tests_parser(void) {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_parser_sync_byte),
    cmocka_unit_test(test_parser_payload_len),
    cmocka_unit_test(test_parser_crc),
    cmocka_unit_test(test_parser_end_frame),
    cmocka_unit_test(test_parser_msg_rep_mode_get),
    cmocka_unit_test(test_parser_msg_rep_mode_set),
    cmocka_unit_test(test_parser_msg_sample),
    cmocka_unit_test(test_parser_msg_dev_id_set),
    cmocka_unit_test(test_parser_msg_sleep_set_on),
    cmocka_unit_test(test_parser_msg_sleep_set_off),
    cmocka_unit_test(test_parser_msg_sleep_get),
    cmocka_unit_test(test_parser_msg_fw_ver_get),
    cmocka_unit_test(test_parser_msg_op_mode_set_periodic),
    cmocka_unit_test(test_parser_msg_op_mode_set_continous),
    cmocka_unit_test(test_parser_msg_op_mode_get),
    cmocka_unit_test(test_parser_invalid_state),
    cmocka_unit_test(test_parser_invalid_msg),
    cmocka_unit_test(test_parser_rep_mode_invalid_data),
    cmocka_unit_test(test_parser_sleep_invalid_data),
    cmocka_unit_test(test_parser_op_mode_invalid_data),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
