/*lint -e708 -e818*/
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "../src/sds011_builder.h"

static void test_builder_params(void **state) {
  (void) state; /* unused */

  uint8_t buffer[20];

  sds011_msg_t msg = {
    .dev_id = 0x1234,
    .type   = SDS011_MSG_TYPE_REP_MODE,
    .op     = SDS011_MSG_OP_GET,
    .src    = SDS011_MSG_SRC_HOST,
    .data   = { 
      .rep_mode = SDS011_REP_MODE_ACTIVE,
    }
  };

  // Invalid params
  assert_int_equal(sds011_builder_build(NULL, buffer, sizeof(buffer)), 0);
  assert_int_equal(sds011_builder_build(&msg, NULL,   sizeof(buffer)), 0);

  // size (query)
  assert_int_equal(sds011_builder_build(&msg, buffer, 18), 0);
  assert_int_equal(sds011_builder_build(&msg, buffer, 19), 19);

  // size (reply)
  msg = (sds011_msg_t) {
    .dev_id        = 0x1234,
    .type          = SDS011_MSG_TYPE_REP_MODE,
    .op            = SDS011_MSG_OP_GET,
    .src           = SDS011_MSG_SRC_SENSOR,
    .data.rep_mode = SDS011_REP_MODE_ACTIVE,
  };

  assert_int_equal(sds011_builder_build(&msg, buffer, 9), 0);
  assert_int_equal(sds011_builder_build(&msg, buffer, 10), 10);
}

static void test_builder_rep_mode_get(void **state) {
  (void) state; /* unused */

  uint8_t buffer[19];

  // host
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id = 0xFFFF,
    .type   = SDS011_MSG_TYPE_REP_MODE,
    .op     = SDS011_MSG_OP_GET,
    .src    = SDS011_MSG_SRC_HOST
  }, buffer, sizeof(buffer)));
  uint8_t ref1[] = {
    0xAA, 0xB4, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0xFF, 0x00, 0xAB
  };
  assert_memory_equal(buffer, ref1, SDS011_QUERY_PACKET_SIZE);

  // sensor
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id        = 0xA160,
    .type          = SDS011_MSG_TYPE_REP_MODE,
    .op            = SDS011_MSG_OP_GET,
    .src           = SDS011_MSG_SRC_SENSOR,
    .data.rep_mode = SDS011_REP_MODE_ACTIVE,
  }, buffer, sizeof(buffer)));
  uint8_t ref2[] = {
    0xAA, 0xC5, 0x02, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x03, 0xAB
  };
  assert_memory_equal(buffer, ref2, SDS011_REPLY_PACKET_SIZE);

  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id        = 0xA160,
    .type          = SDS011_MSG_TYPE_REP_MODE,
    .op            = SDS011_MSG_OP_GET,
    .src           = SDS011_MSG_SRC_SENSOR,
    .data.rep_mode = SDS011_REP_MODE_QUERY,
  }, buffer, sizeof(buffer)));
  uint8_t ref3[] = {
    0xAA, 0xC5, 0x02, 0x00, 0x01, 0x00, 0xA1, 0x60, 0x04, 0xAB
  };
  assert_memory_equal(buffer, ref3, SDS011_REPLY_PACKET_SIZE);
}

static void test_builder_rep_mode_set(void **state) {
  (void) state; /* unused */

  uint8_t buffer[19];

  // host
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id        = 0xA160,
    .type          = SDS011_MSG_TYPE_REP_MODE,
    .op            = SDS011_MSG_OP_SET,
    .src           = SDS011_MSG_SRC_HOST,
    .data.rep_mode = SDS011_REP_MODE_QUERY,
  }, buffer, sizeof(buffer)));
  uint8_t ref1[] = {
    0xAA, 0xB4, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1,
    0x60, 0x05, 0xAB
  };
  assert_memory_equal(buffer, ref1, SDS011_QUERY_PACKET_SIZE);

  // sensor
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id        = 0xA160,
    .type          = SDS011_MSG_TYPE_REP_MODE,
    .op            = SDS011_MSG_OP_SET,
    .src           = SDS011_MSG_SRC_SENSOR,
    .data.rep_mode = SDS011_REP_MODE_QUERY,
  }, buffer, sizeof(buffer)));
  uint8_t ref2[] = {
    0xAA, 0xC5, 0x02, 0x01, 0x01, 0x00, 0xA1, 0x60, 0x05, 0xAB
  };
  assert_memory_equal(buffer, ref2, SDS011_REPLY_PACKET_SIZE);
}

static void test_builder_sample(void **state) {
  (void) state; /* unused */

  uint8_t buffer[19];

  // host
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id               = 0xFFFF,
    .type                 = SDS011_MSG_TYPE_DATA,
    .op                   = SDS011_MSG_OP_GET,
    .src                  = SDS011_MSG_SRC_HOST
  }, buffer, sizeof(buffer)));
  uint8_t ref1[] = {
    0xAA, 0xB4, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
    0xFF, 0x02, 0xAB
  };
  assert_memory_equal(buffer, ref1, SDS011_QUERY_PACKET_SIZE);

  // sensor
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id             = 0xA160,
    .type               = SDS011_MSG_TYPE_DATA,
    .op                 = SDS011_MSG_OP_GET,
    .src                = SDS011_MSG_SRC_SENSOR,
    .data.sample.pm2_5  = 1236,
    .data.sample.pm10   = 2618
  }, buffer, sizeof(buffer)));
  uint8_t ref2[] = {
    0xAA, 0xC0, 0xD4, 0x04, 0x3A, 0x0A, 0xA1, 0x60, 0x1D, 0xAB
  };
  assert_memory_equal(buffer, ref2, SDS011_REPLY_PACKET_SIZE);
}

static void test_builder_dev_id_set(void **state) {
  (void) state; /* unused */

  uint8_t buffer[19];

  // host
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id          = 0xA160,
    .type            = SDS011_MSG_TYPE_DEV_ID,
    .op              = SDS011_MSG_OP_SET,
    .src             = SDS011_MSG_SRC_HOST,
    .data.new_dev_id = 0xA001,
  }, buffer, sizeof(buffer)));
  uint8_t ref1[] = {
    0xAA, 0xB4, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x01, 0xA1,
    0x60, 0xA7, 0xAB
  };
  assert_memory_equal(buffer, ref1, SDS011_QUERY_PACKET_SIZE);

  // sensor
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id             = 0xA001,
    .type               = SDS011_MSG_TYPE_DEV_ID,
    .op                 = SDS011_MSG_OP_SET,
    .src                = SDS011_MSG_SRC_SENSOR,
  }, buffer, sizeof(buffer)));
  uint8_t ref2[] = {
    0xAA, 0xC5, 0x05, 0x00, 0x00, 0x00, 0xA0, 0x01, 0xA6, 0xAB
  };
  assert_memory_equal(buffer, ref2, SDS011_REPLY_PACKET_SIZE);
}

static void test_builder_sleep_set_on(void **state) {
  (void) state; /* unused */

  uint8_t buffer[19];

  // host
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id     = 0xA160,
    .type       = SDS011_MSG_TYPE_SLEEP,
    .op         = SDS011_MSG_OP_SET,
    .src        = SDS011_MSG_SRC_HOST,
    .data.sleep = SDS011_SLEEP_ON
  }, buffer, sizeof(buffer)));
  uint8_t ref1[] = {
    0xAA, 0xB4, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x08, 0xAB
  };
  assert_memory_equal(buffer, ref1, SDS011_QUERY_PACKET_SIZE);

  // sensor
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id     = 0xA160,
    .type       = SDS011_MSG_TYPE_SLEEP,
    .op         = SDS011_MSG_OP_SET,
    .src        = SDS011_MSG_SRC_SENSOR,
    .data.sleep = SDS011_SLEEP_ON,
  }, buffer, sizeof(buffer)));
  uint8_t ref2[] = {
    0xAA, 0xC5, 0x06, 0x01, 0x00, 0x00, 0xA1, 0x60, 0x08, 0xAB
  };
  assert_memory_equal(buffer, ref2, SDS011_REPLY_PACKET_SIZE);
}

static void test_builder_sleep_set_off(void **state) {
  (void) state; /* unused */

  uint8_t buffer[19];

  // host
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id     = 0xA160,
    .type       = SDS011_MSG_TYPE_SLEEP,
    .op         = SDS011_MSG_OP_SET,
    .src        = SDS011_MSG_SRC_HOST,
    .data.sleep = SDS011_SLEEP_OFF
  }, buffer, sizeof(buffer)));
  uint8_t ref1[] = {
    0xAA, 0xB4, 0x06, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x09, 0xAB
  };
  assert_memory_equal(buffer, ref1, SDS011_QUERY_PACKET_SIZE);

  // sensor
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id     = 0xA160,
    .type       = SDS011_MSG_TYPE_SLEEP,
    .op         = SDS011_MSG_OP_SET,
    .src        = SDS011_MSG_SRC_SENSOR,
    .data.sleep = SDS011_SLEEP_OFF,
  }, buffer, sizeof(buffer)));
  uint8_t ref2[] = {
    0xAA, 0xC5, 0x06, 0x01, 0x01, 0x00, 0xA1, 0x60, 0x09, 0xAB
  };
  assert_memory_equal(buffer, ref2, SDS011_REPLY_PACKET_SIZE);
}

static void test_builder_sleep_get(void **state) {
  (void) state; /* unused */

  uint8_t buffer[19];

  // host
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id           = 0xA160,
    .type             = SDS011_MSG_TYPE_SLEEP,
    .op               = SDS011_MSG_OP_GET,
    .src              = SDS011_MSG_SRC_HOST,
  }, buffer, sizeof(buffer)));
  uint8_t ref1[] = {
    0xAA, 0xB4, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x07, 0xAB
  };
  assert_memory_equal(buffer, ref1, SDS011_QUERY_PACKET_SIZE);

  // sensor
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id     = 0xA160,
    .type       = SDS011_MSG_TYPE_SLEEP,
    .op         = SDS011_MSG_OP_GET,
    .src        = SDS011_MSG_SRC_SENSOR,
    .data.sleep = SDS011_SLEEP_OFF
  }, buffer, sizeof(buffer)));
  uint8_t ref2[] = {
    0xAA, 0xC5, 0x06, 0x00, 0x01, 0x00, 0xA1, 0x60, 0x08, 0xAB
  };
  assert_memory_equal(buffer, ref2, SDS011_REPLY_PACKET_SIZE);

  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id     = 0xA160,
    .type       = SDS011_MSG_TYPE_SLEEP,
    .op         = SDS011_MSG_OP_GET,
    .src        = SDS011_MSG_SRC_SENSOR,
    .data.sleep = SDS011_SLEEP_ON
  }, buffer, sizeof(buffer)));
  uint8_t ref3[] = {
    0xAA, 0xC5, 0x06, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x07, 0xAB
  };
  assert_memory_equal(buffer, ref3, SDS011_REPLY_PACKET_SIZE);
}

static void test_builder_fw_ver(void **state) {
  (void) state; /* unused */

  uint8_t buffer[19];

  // host
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id               = 0xA160,
    .type                 = SDS011_MSG_TYPE_FW_VER,
    .op                   = SDS011_MSG_OP_GET,
    .src                  = SDS011_MSG_SRC_HOST,
  }, buffer, sizeof(buffer)));
  uint8_t ref1[] = {
    0xAA, 0xB4, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x08, 0xAB
  };
  assert_memory_equal(buffer, ref1, SDS011_QUERY_PACKET_SIZE);

  // sensor
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id             = 0xA160,
    .type               = SDS011_MSG_TYPE_FW_VER,
    .op                 = SDS011_MSG_OP_GET,
    .src                = SDS011_MSG_SRC_SENSOR,
    .data.fw_ver.year   = 15,
    .data.fw_ver.month  = 7,
    .data.fw_ver.day    = 10,
  }, buffer, sizeof(buffer)));
  uint8_t ref2[] = {
    0xAA, 0xC5, 0x07, 0x0F, 0x07, 0x0A, 0xA1, 0x60, 0x28, 0xAB
  };
  assert_memory_equal(buffer, ref2, SDS011_REPLY_PACKET_SIZE);
}

static void test_builder_op_mode_set_periodic(void **state) {
  (void) state; /* unused */

  uint8_t buffer[19];

  // host
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id                 = 0xA160,
    .type                   = SDS011_MSG_TYPE_OP_MODE,
    .op                     = SDS011_MSG_OP_SET,
    .src                    = SDS011_MSG_SRC_HOST,
    .data.op_mode.mode      = SDS011_OP_MODE_INTERVAL,
    .data.op_mode.interval  = 1,
  }, buffer, sizeof(buffer)));
  uint8_t ref1[] = {
    0xAA, 0xB4, 0x08, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x0B, 0xAB
  };
  assert_memory_equal(buffer, ref1, SDS011_QUERY_PACKET_SIZE);

  // sensor
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id                 = 0xA160,
    .type                   = SDS011_MSG_TYPE_OP_MODE,
    .op                     = SDS011_MSG_OP_SET,
    .src                    = SDS011_MSG_SRC_SENSOR,
    .data.op_mode.mode      = SDS011_OP_MODE_INTERVAL,
    .data.op_mode.interval  = 1,
  }, buffer, sizeof(buffer)));
  uint8_t ref2[] = {
    0xAA, 0xC5, 0x08, 0x01, 0x01, 0x00, 0xA1, 0x60, 0x0B, 0xAB
  };
  assert_memory_equal(buffer, ref2, SDS011_REPLY_PACKET_SIZE);
}

static void test_builder_op_mode_set_continous(void **state) {
  (void) state; /* unused */

  uint8_t buffer[19];

  // host
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id                 = 0xA160,
    .type                   = SDS011_MSG_TYPE_OP_MODE,
    .op                     = SDS011_MSG_OP_SET,
    .src                    = SDS011_MSG_SRC_HOST,
    .data.op_mode.mode      = SDS011_OP_MODE_CONTINOUS,
    .data.op_mode.interval  = 0,
  }, buffer, sizeof(buffer)));
  uint8_t ref1[] = {
    0xAA, 0xB4, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x0A, 0xAB
  };
  assert_memory_equal(buffer, ref1, SDS011_QUERY_PACKET_SIZE);

  // sensor
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id                 = 0xA160,
    .type                   = SDS011_MSG_TYPE_OP_MODE,
    .op                     = SDS011_MSG_OP_SET,
    .src                    = SDS011_MSG_SRC_SENSOR,
    .data.op_mode.mode      = SDS011_OP_MODE_CONTINOUS,
    .data.op_mode.interval  = 0,
  }, buffer, sizeof(buffer)));
  uint8_t ref2[] = {
    0xAA, 0xC5, 0x08, 0x01, 0x00, 0x00, 0xA1, 0x60, 0x0A, 0xAB
  };
  assert_memory_equal(buffer, ref2, SDS011_REPLY_PACKET_SIZE);
}

static void test_builder_op_mode_get(void **state) {
  (void) state; /* unused */

  uint8_t buffer[19];

  // host
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id           = 0xA160,
    .type             = SDS011_MSG_TYPE_OP_MODE,
    .op               = SDS011_MSG_OP_GET,
    .src              = SDS011_MSG_SRC_HOST,
  }, buffer, sizeof(buffer)));
  uint8_t ref1[] = {
    0xAA, 0xB4, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x09, 0xAB
  };
  assert_memory_equal(buffer, ref1, SDS011_QUERY_PACKET_SIZE);

  // sensor
  assert_true(sds011_builder_build(&(sds011_msg_t) {
    .dev_id                 = 0xA160,
    .type                   = SDS011_MSG_TYPE_OP_MODE,
    .op                     = SDS011_MSG_OP_GET,
    .src                    = SDS011_MSG_SRC_SENSOR,
    .data.op_mode.mode      = SDS011_OP_MODE_INTERVAL,
    .data.op_mode.interval  = 2,
  }, buffer, sizeof(buffer)));
  uint8_t ref2[] = {
    0xAA, 0xC5, 0x08, 0x00, 0x02, 0x00, 0xA1, 0x60, 0x0B, 0xAB
  };
  assert_memory_equal(buffer, ref2, SDS011_REPLY_PACKET_SIZE);
}

static void test_builder_missing(void **state) {
  (void)state;

  uint8_t buffer[SDS011_REPLY_PACKET_SIZE];
  assert_int_equal(sds011_builder_build(&(sds011_msg_t) {
    .dev_id                 = 0xA160,
    .type                   = (sds011_msg_type_t)0,
    .op                     = SDS011_MSG_OP_GET,
    .src                    = SDS011_MSG_SRC_SENSOR,
    .data.op_mode.mode      = SDS011_OP_MODE_INTERVAL,
    .data.op_mode.interval  = 2,
  }, buffer, sizeof(buffer)), 0);

  assert_int_equal(sds011_builder_get_error(), SDS011_ERR_INVALID_MSG_TYPE);
}

static void test_builder_invalid_type(void **state) {
  (void)state;

  uint8_t buffer[SDS011_REPLY_PACKET_SIZE];
  assert_int_equal(sds011_builder_build(&(sds011_msg_t) {
    .dev_id                 = 0xA160,
    .type                   = SDS011_MSG_TYPE_COUNT,
    .op                     = SDS011_MSG_OP_GET,
    .src                    = SDS011_MSG_SRC_SENSOR,
    .data.op_mode.mode      = SDS011_OP_MODE_INTERVAL,
    .data.op_mode.interval  = 2,
  }, buffer, sizeof(buffer)), 0);

  assert_int_equal(sds011_builder_get_error(), SDS011_ERR_INVALID_MSG_TYPE);
}

static void test_builder_invalid_src(void **state) {
  (void)state;

  uint8_t buffer[SDS011_REPLY_PACKET_SIZE];
  assert_int_equal(sds011_builder_build(&(sds011_msg_t) {
    .dev_id                 = 0xA160,
    .type                   = SDS011_MSG_TYPE_OP_MODE,
    .op                     = SDS011_MSG_OP_GET,
    .src                    = (sds011_msg_src_t)0x7f,
    .data.op_mode.mode      = SDS011_OP_MODE_INTERVAL,
    .data.op_mode.interval  = 2,
  }, buffer, sizeof(buffer)), 0);

  assert_int_equal(sds011_builder_get_error(), SDS011_ERR_INVALID_SRC);
}

int tests_builder(void) {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_builder_params),
    cmocka_unit_test(test_builder_rep_mode_get),
    cmocka_unit_test(test_builder_rep_mode_set),
    cmocka_unit_test(test_builder_sample),
    cmocka_unit_test(test_builder_dev_id_set),
    cmocka_unit_test(test_builder_sleep_set_on),
    cmocka_unit_test(test_builder_sleep_set_off),
    cmocka_unit_test(test_builder_sleep_get),
    cmocka_unit_test(test_builder_fw_ver),
    cmocka_unit_test(test_builder_op_mode_set_periodic),
    cmocka_unit_test(test_builder_op_mode_set_continous),
    cmocka_unit_test(test_builder_op_mode_get),
    cmocka_unit_test(test_builder_missing),
    cmocka_unit_test(test_builder_invalid_type),
    cmocka_unit_test(test_builder_invalid_src),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
