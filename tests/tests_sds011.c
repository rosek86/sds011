#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>

#include "../src/sds011.h"

static uint32_t _millis = 0;
static uint32_t millis_mock(void) {
  return _millis;
}

static uint32_t _bytes_available = 0;
static size_t bytes_available_mock(void *user_data) {
  (void)user_data;
  return _bytes_available;
}

static uint32_t read_byte_iter = 0;
static uint8_t read_byte_buffer[32];
static uint8_t read_byte_mock(void *user_data) {
  (void)user_data;
  if (_bytes_available > 0) {
    _bytes_available--;
  }
  if (read_byte_iter < sizeof(read_byte_buffer)) {
    return read_byte_buffer[read_byte_iter++];
  }
  return 0;
}

static uint32_t send_byte_iter = 0;
static uint32_t _send_bytes_available = 0;
static uint8_t send_byte_buffer[32];
static bool send_byte_mock(uint8_t byte, void *user_data) {
  (void)user_data;
  if (send_byte_iter < _send_bytes_available) {
    send_byte_buffer[send_byte_iter++] = byte;
  }
  return true;
}

// helper initialization function
static void init_sds011(sds011_t *sds011) {
  assert_int_equal(sds011_init(sds011, &(sds011_init_t) {
    .msg_timeout = 1000,
    .retries = 2,
    .millis = millis_mock,
    .serial = {
      .bytes_available  = bytes_available_mock,
      .read_byte        = read_byte_mock,
      .send_byte        = send_byte_mock
    },
  }), SDS011_OK);
}

static void test_init(void **state) {
  (void) state; /* unused */

  sds011_t sds011;

  assert_int_equal(sds011_init(NULL, NULL), SDS011_ERR_INVALID_PARAM);
  assert_int_equal(sds011_init(&sds011, NULL), SDS011_ERR_INVALID_PARAM);
  assert_int_equal(sds011_init(&sds011, &(sds011_init_t) {
    .millis = NULL
  }), SDS011_ERR_INVALID_PARAM);
  assert_int_equal(sds011_init(&sds011, &(sds011_init_t) {
    .millis = millis_mock,
    .serial = {
      .bytes_available = NULL,
    }
  }), SDS011_ERR_INVALID_PARAM);
  assert_int_equal(sds011_init(&sds011, &(sds011_init_t) {
    .millis = millis_mock,
    .serial = {
      .bytes_available = bytes_available_mock,
      .read_byte = NULL,
    }
  }), SDS011_ERR_INVALID_PARAM);
  assert_int_equal(sds011_init(&sds011, &(sds011_init_t) {
    .millis = millis_mock,
    .serial = {
      .bytes_available = bytes_available_mock,
      .read_byte = read_byte_mock,
      .send_byte = NULL
    }
  }), SDS011_ERR_INVALID_PARAM);
  assert_int_equal(sds011_init(&sds011, &(sds011_init_t) {
    .millis = millis_mock,
    .serial = {
      .bytes_available = bytes_available_mock,
      .read_byte = read_byte_mock,
      .send_byte = send_byte_mock
    },
  }), SDS011_OK);
}

static void on_sample_callback(sds011_msg_t const *msg, void *user_data) {
  (void)msg;
  (void)user_data;
}

static void test_query_data(void **state) {
  (void) state; /* unused */

  sds011_t sds011;
  init_sds011(&sds011);

  assert_int_equal(sds011_query_data(NULL, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  _send_bytes_available = sizeof(send_byte_buffer);
  assert_int_equal(sds011_query_data(&sds011, 0xFFFF, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  assert_int_equal(sds011_process(&sds011), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x02, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);

  size_t size = sds011_builder_build(&(sds011_msg_t) {
    .dev_id             = 0xA160,
    .type               = SDS011_MSG_TYPE_DATA,
    .op                 = SDS011_MSG_OP_GET,
    .src                = SDS011_MSG_SRC_SENSOR,
    .data.sample.pm2_5  = 1236,
    .data.sample.pm10   = 2618,
  }, read_byte_buffer, sizeof(read_byte_buffer));

  // reply without callback
  _bytes_available = size;
  read_byte_iter = 0;
  sds011.on_sample.callback = NULL;
  assert_int_equal(sds011_process(&sds011), SDS011_OK);

  // reply with callback
  _bytes_available = size;
  read_byte_iter = 0;
  sds011.on_sample.callback = on_sample_callback;
  assert_int_equal(sds011_process(&sds011), SDS011_OK);
}

static void test_max_requests(void **state) {
  (void) state; /* unused */

  sds011_t sds011;
  init_sds011(&sds011);

  for (int i = 0; i < SDS011_REQ_QUEUE_SIZE; i++) {
    assert_int_equal(sds011_query_data(&sds011, 0xFFFF, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  }
  assert_int_equal(sds011_query_data(&sds011, 0xFFFF, (sds011_cb_t){NULL, NULL}), SDS011_ERR_BUSY);
}

static void test_set_dev_id(void **state) {
  (void) state; /* unused */

  sds011_t sds011;
  init_sds011(&sds011);

  assert_int_equal(sds011_set_device_id(NULL, 0, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  _send_bytes_available = sizeof(send_byte_buffer);
  assert_int_equal(sds011_set_device_id(&sds011, 0xA160, 0xA001, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  assert_int_equal(sds011_process(&sds011), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xA0, 0x01, 0xA1, 0x60, 0xA7, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);

  _bytes_available = sds011_builder_build(&(sds011_msg_t) {
    .dev_id           = 0xA001,
    .type             = SDS011_MSG_TYPE_DEV_ID,
    .op               = SDS011_MSG_OP_GET,
    .src              = SDS011_MSG_SRC_SENSOR,
  }, read_byte_buffer, sizeof(read_byte_buffer));

  read_byte_iter = 0;
  assert_int_equal(sds011_process(&sds011), SDS011_OK); // first send

  // invalid reply
  send_byte_iter = 0;
  _send_bytes_available = sizeof(send_byte_buffer);
  assert_int_equal(sds011_set_device_id(&sds011, 0xA160, 0xA001, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  assert_int_equal(sds011_process(&sds011), SDS011_OK);

  _bytes_available = sds011_builder_build(&(sds011_msg_t) {
    .dev_id           = 0xFFFF, // invalid id
    .type             = SDS011_MSG_TYPE_DEV_ID,
    .op               = SDS011_MSG_OP_GET,
    .src              = SDS011_MSG_SRC_SENSOR,
  }, read_byte_buffer, sizeof(read_byte_buffer));

  read_byte_iter = 0;
  assert_int_equal(sds011_process(&sds011), SDS011_OK); // first send
}

static void test_set_reporting_active(void **state) {
  (void) state; /* unused */

  sds011_t sds011;
  init_sds011(&sds011);

  assert_int_equal(sds011_set_reporting_mode_active(NULL, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  _send_bytes_available = sizeof(send_byte_buffer);
  assert_int_equal(sds011_set_reporting_mode_active(&sds011, 0xA160, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  assert_int_equal(sds011_process(&sds011), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x04, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

static void test_set_reporting_query(void **state) {
  (void) state; /* unused */

  sds011_t sds011;
  init_sds011(&sds011);

  assert_int_equal(sds011_set_reporting_mode_query(NULL, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  _send_bytes_available = sizeof(send_byte_buffer);
  assert_int_equal(sds011_set_reporting_mode_query(&sds011, 0xA160, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  assert_int_equal(sds011_process(&sds011), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x05, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

static void test_get_reporting(void **state) {
  (void) state; /* unused */

  sds011_t sds011;
  init_sds011(&sds011);

  assert_int_equal(sds011_get_reporting_mode(NULL, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  _send_bytes_available = sizeof(send_byte_buffer);
  assert_int_equal(sds011_get_reporting_mode(&sds011, 0xFFFF, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  assert_int_equal(sds011_process(&sds011), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

static void test_set_sleep_on(void **state) {
  (void) state; /* unused */

  sds011_t sds011;
  init_sds011(&sds011);

  assert_int_equal(sds011_set_sleep_on(NULL, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  _send_bytes_available = sizeof(send_byte_buffer);
  assert_int_equal(sds011_set_sleep_on(&sds011, 0xA160, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  assert_int_equal(sds011_process(&sds011), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x08, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

static void test_set_sleep_off(void **state) {
  (void) state; /* unused */

  sds011_t sds011;
  init_sds011(&sds011);

  assert_int_equal(sds011_set_sleep_off(NULL, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  _send_bytes_available = sizeof(send_byte_buffer);
  assert_int_equal(sds011_set_sleep_off(&sds011, 0xA160, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  assert_int_equal(sds011_process(&sds011), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x06, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x09, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

static void test_get_sleep(void **state) {
  (void) state; /* unused */

  sds011_t sds011;
  init_sds011(&sds011);

  assert_int_equal(sds011_get_sleep(NULL, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  _send_bytes_available = sizeof(send_byte_buffer);
  assert_int_equal(sds011_get_sleep(&sds011, 0xA160, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  assert_int_equal(sds011_process(&sds011), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x07, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

static void test_set_op_mode_continous(void **state) {
  (void) state; /* unused */

  sds011_t sds011;
  init_sds011(&sds011);

  assert_int_equal(sds011_set_op_mode_continous(NULL, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  _send_bytes_available = sizeof(send_byte_buffer);
  assert_int_equal(sds011_set_op_mode_continous(&sds011, 0xA160, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  assert_int_equal(sds011_process(&sds011), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x0A, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

static void test_set_op_mode_periodic(void **state) {
  (void) state; /* unused */

  sds011_t sds011;
  init_sds011(&sds011);

  assert_int_equal(sds011_set_op_mode_periodic(NULL, 0, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  _send_bytes_available = sizeof(send_byte_buffer);
  assert_int_equal(sds011_set_op_mode_periodic(&sds011, 0xA160, 1, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  assert_int_equal(sds011_process(&sds011), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x08, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x0B, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

static void test_get_op_mode(void **state) {
  (void) state; /* unused */

  sds011_t sds011;
  init_sds011(&sds011);

  assert_int_equal(sds011_get_op_mode(NULL, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  _send_bytes_available = sizeof(send_byte_buffer);
  assert_int_equal(sds011_get_op_mode(&sds011, 0xA160, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  assert_int_equal(sds011_process(&sds011), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x09, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

static void test_get_fw_ver(void **state) {
  (void) state; /* unused */

  sds011_t sds011;
  init_sds011(&sds011);

  assert_int_equal(sds011_get_fw_ver(NULL, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  _send_bytes_available = sizeof(send_byte_buffer);
  assert_int_equal(sds011_get_fw_ver(&sds011, 0xA160, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  assert_int_equal(sds011_process(&sds011), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x08, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

static uint32_t _cmd_cb_call_cnt;
static sds011_err_t _cmd_cb_err;
static sds011_msg_t const *_cmd_cb_msg;
static void cmd_callback(sds011_err_t err, sds011_msg_t const *msg, void *user_data) {
  (void) user_data;

  _cmd_cb_call_cnt++;
  _cmd_cb_err = err;
  _cmd_cb_msg = msg;
}

static void test_validate_set_result(void **state) {
  (void) state; /* unused */

  // send interval value 1 but receive interval value 2

  sds011_t sds011;
  assert_int_equal(sds011_init(&sds011, &(sds011_init_t) {
    .retries = 2,
    .millis = millis_mock,
    .serial = {
      .bytes_available  = bytes_available_mock,
      .read_byte        = read_byte_mock,
      .send_byte        = send_byte_mock
    },
  }), SDS011_OK);

  assert_int_equal(sds011_set_op_mode_periodic(&sds011, 0xA160, 1, (sds011_cb_t) {
    .callback = cmd_callback,
    .user_data = NULL,
  }), SDS011_OK);

  size_t size = sds011_builder_build(&(sds011_msg_t) {
    .dev_id                 = 0xA160,
    .type                   = SDS011_MSG_TYPE_OP_MODE,
    .op                     = SDS011_MSG_OP_SET,
    .src                    = SDS011_MSG_SRC_SENSOR,
    .data.op_mode.mode      = SDS011_OP_MODE_INTERVAL,
    .data.op_mode.interval  = 2,
  }, read_byte_buffer, sizeof(read_byte_buffer));
  assert_true(size > 0);
  assert_int_equal(sds011_process(&sds011), SDS011_OK); // first send

  read_byte_iter = 0;
  _cmd_cb_call_cnt = 0;
  _bytes_available = size;

  assert_int_equal(sds011_process(&sds011), SDS011_OK); // second send
  assert_int_equal(_cmd_cb_call_cnt, 0);

  read_byte_iter = 0;
  _cmd_cb_call_cnt = 0;
  _bytes_available = size;

  assert_int_equal(sds011_process(&sds011), SDS011_OK);
  assert_int_equal(_cmd_cb_call_cnt, 1);
  assert_int_not_equal(_cmd_cb_err, SDS011_OK);
}

static void test_dev_id_ffff_is_accepted(void **state) {
  (void) state;

  // send request with 0xFFFF id, reply with 0xA160. Callback should be executed.

  sds011_t sds011;
  init_sds011(&sds011);

  // Build reply
  size_t size = sds011_builder_build(&(sds011_msg_t) {
    .dev_id                 = 0xA160,
    .type                   = SDS011_MSG_TYPE_OP_MODE,
    .op                     = SDS011_MSG_OP_SET,
    .src                    = SDS011_MSG_SRC_SENSOR,
    .data.op_mode.mode      = SDS011_OP_MODE_INTERVAL,
    .data.op_mode.interval  = 1,
  }, read_byte_buffer, sizeof(read_byte_buffer));
  assert_true(size > 0);

  // send request
  assert_int_equal(sds011_set_op_mode_periodic(&sds011, 0xFFFF, 1, (sds011_cb_t) {
    .callback = cmd_callback,
    .user_data = NULL,
  }), SDS011_OK);
  assert_int_equal(sds011_process(&sds011), SDS011_OK);

  // Process
  read_byte_iter = 0;
  _cmd_cb_call_cnt = 0;
  _bytes_available = size;

  assert_int_equal(sds011_process(&sds011), SDS011_OK);
  assert_int_equal(_cmd_cb_call_cnt, 1);
  assert_int_equal(_cmd_cb_err, SDS011_OK);
}

static void test_queue(void **state) {
  (void) state;

  // send request with 0xFFFF id, reply with 0xA160. Callback should be executed.

  sds011_t sds011;
  init_sds011(&sds011);

  // Build reply
  size_t size = sds011_builder_build(&(sds011_msg_t) {
    .dev_id                 = 0xA160,
    .type                   = SDS011_MSG_TYPE_OP_MODE,
    .op                     = SDS011_MSG_OP_SET,
    .src                    = SDS011_MSG_SRC_SENSOR,
    .data.op_mode.mode      = SDS011_OP_MODE_INTERVAL,
    .data.op_mode.interval  = 1,
  }, read_byte_buffer, sizeof(read_byte_buffer));
  assert_true(size > 0);

  // send request
  _cmd_cb_call_cnt = 0;

  assert_int_equal(sds011_set_op_mode_periodic(&sds011, 0xFFFF, 1, (sds011_cb_t) {
    .callback = cmd_callback,
    .user_data = NULL,
  }), SDS011_OK);
  assert_int_equal(sds011_set_op_mode_periodic(&sds011, 0xFFFF, 1, (sds011_cb_t) {
    .callback = cmd_callback,
    .user_data = NULL,
  }), SDS011_OK);

  assert_int_equal(_cmd_cb_call_cnt, 0);
}

static void test_set_callback(void **state) {
  (void)state; // unused

  assert_int_equal(sds011_set_sample_callback(
    NULL, (sds011_on_sample_t) {}), SDS011_ERR_INVALID_PARAM);

  sds011_t sds011;
  assert_int_equal(sds011_set_sample_callback(
    &sds011, (sds011_on_sample_t) {}), SDS011_OK);
}

static void test_process_error(void **state) {
  (void)state;

  sds011_t sds011;
  init_sds011(&sds011);

  uint8_t buf[] = { 0xAA, 0xC5, 0x08, 0x00, 31, 0x00, 0xA1, 0x60, 0x28, 0xAB };
  memcpy(read_byte_buffer, buf, sizeof(buf));

  _bytes_available = sizeof(buf);
  read_byte_iter = 0;

  assert_int_equal(sds011_process(&sds011), SDS011_ERR_INVALID_DATA);
}

static void test_process_split(void **state) {
  (void)state;

  sds011_t sds011;
  init_sds011(&sds011);

  size_t size = sds011_builder_build(&(sds011_msg_t) {
    .dev_id                 = 0xA160,
    .type                   = SDS011_MSG_TYPE_OP_MODE,
    .op                     = SDS011_MSG_OP_SET,
    .src                    = SDS011_MSG_SRC_SENSOR,
    .data.op_mode.mode      = SDS011_OP_MODE_INTERVAL,
    .data.op_mode.interval  = 2,
  }, read_byte_buffer, sizeof(read_byte_buffer));

  read_byte_iter = 0;

  _bytes_available = size - 5;
  assert_int_equal(sds011_process(&sds011), SDS011_OK);

  _bytes_available = 5;
  assert_int_equal(sds011_process(&sds011), SDS011_OK);
}

static void test_request_timeout(void **state) {
  (void)state;

  sds011_t sds011;
  init_sds011(&sds011);

  send_byte_iter = 0;
  _send_bytes_available = SDS011_QUERY_PACKET_SIZE - 2;
  read_byte_iter = 0;
  _bytes_available = 0;
  _millis = 0;

  assert_int_equal(sds011_query_data(&sds011, 0xFFFF, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  assert_int_equal(sds011_process(&sds011), SDS011_OK); // send

  // without send timeout
  assert_int_equal(sds011_process(&sds011), SDS011_OK); // check for timeout

  // with send timeout
  _millis = 5000;
  assert_int_equal(sds011_process(&sds011), SDS011_OK); // check for timeout
}

static void test_infinite_send_timeout(void **state) {
  (void)state;

  sds011_t sds011;
  init_sds011(&sds011);
  sds011.cfg.msg_timeout = 0;

  send_byte_iter = 0;
  _send_bytes_available = SDS011_QUERY_PACKET_SIZE - 1;
  read_byte_iter = 0;
  _bytes_available = 0;
  _millis = 0;

  assert_int_equal(sds011_query_data(&sds011, 0xFFFF, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  assert_int_equal(sds011_process(&sds011), SDS011_OK); // send
  assert_int_equal(sds011_process(&sds011), SDS011_OK); // check for timeout
}

static bool send_one_byte_then_timeout_mock(uint8_t byte, void *user_data) {
  (void)user_data;
  if (send_byte_iter < _send_bytes_available) {
    send_byte_buffer[send_byte_iter++] = byte;
  }
  _millis = 5000;
  return false;
}

static void test_send_timeout(void **state) {
  (void)state;

  sds011_t sds011;
  init_sds011(&sds011);
  sds011.cfg.msg_timeout = 1000;
  sds011.cfg.serial.send_byte = send_one_byte_then_timeout_mock;

  send_byte_iter = 0;
  _send_bytes_available = SDS011_QUERY_PACKET_SIZE - 3;
  read_byte_iter = 0;
  _bytes_available = 0;
  _millis = 0;

  assert_int_equal(sds011_query_data(&sds011, 0xFFFF, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  assert_int_equal(sds011_process(&sds011), SDS011_OK); // send
}

static bool _msg_cb_called = false;
static void msg_cb(sds011_err_t err, sds011_msg_t const *msg, void *user_data) {
  (void)err;
  (void)msg;
  (void)user_data;
  _msg_cb_called = true;
}

static void test_send_invalid_msg(void **state) {
  (void) state; /* unused */

  sds011_t sds011;
  init_sds011(&sds011);

  send_byte_iter = 0;
  _send_bytes_available = sizeof(send_byte_buffer);

  sds011_fifo_push(&sds011.req.queue, &(sds011_request_t) {
    .msg = (sds011_msg_t) {
      .dev_id                 = 0xA160,
      .type                   = 500,
      .op                     = SDS011_MSG_OP_SET,
      .src                    = SDS011_MSG_SRC_SENSOR,
      .data.op_mode.mode      = SDS011_OP_MODE_INTERVAL,
      .data.op_mode.interval  = 2,
    },
    .cb = (sds011_cb_t) {
      msg_cb, NULL
    },
  });
  assert_int_equal(sds011_process(&sds011), SDS011_OK);

  _msg_cb_called = false;
  assert_int_equal(sds011_process(&sds011), SDS011_OK);
  assert_true(_msg_cb_called);
}

static void test_other_msg_type_during_request(void **state) {
  (void) state; /* unused */

  sds011_t sds011;
  init_sds011(&sds011);

  send_byte_iter = 0;
  _send_bytes_available = SDS011_QUERY_PACKET_SIZE;

  assert_int_equal(sds011_set_device_id(&sds011, 0xA160, 0xA001, (sds011_cb_t) {
    msg_cb, NULL
  }), SDS011_OK);
  assert_int_equal(sds011_process(&sds011), SDS011_OK);

  _bytes_available = sds011_builder_build(&(sds011_msg_t) {
    .dev_id     = 0xA001,
    .type       = SDS011_MSG_TYPE_SLEEP,
    .op         = SDS011_MSG_OP_GET,
    .src        = SDS011_MSG_SRC_SENSOR,
    .data.sleep = SDS011_SLEEP_ON
  }, read_byte_buffer, sizeof(read_byte_buffer));

  _msg_cb_called = false;
  read_byte_iter = 0;
  assert_int_equal(sds011_process(&sds011), SDS011_OK);
  assert_false(_msg_cb_called);
}

static void test_other_msg_op_during_request(void **state) {
  (void)state;

  sds011_t sds011;
  init_sds011(&sds011);

  send_byte_iter = 0;
  _send_bytes_available = SDS011_QUERY_PACKET_SIZE;

  // set here
  assert_int_equal(sds011_set_op_mode_continous(&sds011, 0xFFFF, (sds011_cb_t) {
    msg_cb, NULL
  }), SDS011_OK);
  assert_int_equal(sds011_process(&sds011), SDS011_OK);

  // get here
  _bytes_available = sds011_builder_build(&(sds011_msg_t) {
    .dev_id     = 0xA001,
    .type       = SDS011_MSG_TYPE_OP_MODE,
    .op         = SDS011_MSG_OP_GET,
    .src        = SDS011_MSG_SRC_SENSOR,
  }, read_byte_buffer, sizeof(read_byte_buffer));

  _msg_cb_called = false;
  read_byte_iter = 0;
  assert_int_equal(sds011_process(&sds011), SDS011_OK);
  assert_false(_msg_cb_called);
}

int tests_sds011(void) {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_init),
    cmocka_unit_test(test_query_data),
    cmocka_unit_test(test_max_requests),
    cmocka_unit_test(test_set_dev_id),
    cmocka_unit_test(test_set_reporting_active),
    cmocka_unit_test(test_set_reporting_query),
    cmocka_unit_test(test_get_reporting),
    cmocka_unit_test(test_set_sleep_on),
    cmocka_unit_test(test_set_sleep_off),
    cmocka_unit_test(test_get_sleep),
    cmocka_unit_test(test_set_op_mode_continous),
    cmocka_unit_test(test_set_op_mode_periodic),
    cmocka_unit_test(test_get_op_mode),
    cmocka_unit_test(test_get_fw_ver),
    cmocka_unit_test(test_validate_set_result),
    cmocka_unit_test(test_dev_id_ffff_is_accepted),
    cmocka_unit_test(test_queue),
    cmocka_unit_test(test_set_callback),
    cmocka_unit_test(test_process_error),
    cmocka_unit_test(test_process_split),
    cmocka_unit_test(test_request_timeout),
    cmocka_unit_test(test_infinite_send_timeout),
    cmocka_unit_test(test_send_timeout),
    cmocka_unit_test(test_send_invalid_msg),
    cmocka_unit_test(test_other_msg_type_during_request),
    cmocka_unit_test(test_other_msg_op_during_request),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
