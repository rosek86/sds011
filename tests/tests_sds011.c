#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>

#include "../src/sds011.h"

static uint32_t millis_mock(void) {
  return 0;
}

static size_t bytes_available_mock(void *user_data) {
  return 0;
}

static uint8_t read_byte_mock(void *user_data) {
  return 0;
}

static uint32_t send_byte_iter = 0;
static uint8_t send_byte_buffer[32];
static bool send_byte_mock(uint8_t byte, void *user_data) {
  if (send_byte_iter < sizeof(send_byte_buffer)) {
    send_byte_buffer[send_byte_iter++] = byte;
  }
  return true;
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

static void test_query_data(void **state) {
  (void) state; /* unused */

  sds011_t sds011;

  assert_int_equal(sds011_init(&sds011, &(sds011_init_t) {
    .millis = millis_mock,
    .serial = {
      .bytes_available = bytes_available_mock,
      .read_byte = read_byte_mock,
      .send_byte = send_byte_mock
    },
  }), SDS011_OK);

  assert_int_equal(sds011_query_data(NULL, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  assert_int_equal(sds011_query_data(&sds011, 0xFFFF, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x02, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

static void test_set_dev_id(void **state) {
  (void) state; /* unused */

  sds011_t sds011;

  assert_int_equal(sds011_init(&sds011, &(sds011_init_t) {
    .millis = millis_mock,
    .serial = {
      .bytes_available = bytes_available_mock,
      .read_byte = read_byte_mock,
      .send_byte = send_byte_mock
    },
  }), SDS011_OK);

  assert_int_equal(sds011_set_device_id(NULL, 0, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  assert_int_equal(sds011_set_device_id(&sds011, 0xA160, 0xA001, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xA0, 0x01, 0xA1, 0x60, 0xA7, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

static void test_set_reporting_active(void **state) {
  (void) state; /* unused */

  sds011_t sds011;

  assert_int_equal(sds011_init(&sds011, &(sds011_init_t) {
    .millis = millis_mock,
    .serial = {
      .bytes_available = bytes_available_mock,
      .read_byte = read_byte_mock,
      .send_byte = send_byte_mock
    },
  }), SDS011_OK);

  assert_int_equal(sds011_set_reporting_mode_active(NULL, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  assert_int_equal(sds011_set_reporting_mode_active(&sds011, 0xA160, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x04, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

static void test_set_reporting_query(void **state) {
  (void) state; /* unused */

  sds011_t sds011;

  assert_int_equal(sds011_init(&sds011, &(sds011_init_t) {
    .millis = millis_mock,
    .serial = {
      .bytes_available = bytes_available_mock,
      .read_byte = read_byte_mock,
      .send_byte = send_byte_mock
    },
  }), SDS011_OK);

  assert_int_equal(sds011_set_reporting_mode_query(NULL, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  assert_int_equal(sds011_set_reporting_mode_query(&sds011, 0xA160, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x05, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

static void test_get_reporting(void **state) {
  (void) state; /* unused */

  sds011_t sds011;

  assert_int_equal(sds011_init(&sds011, &(sds011_init_t) {
    .millis = millis_mock,
    .serial = {
      .bytes_available = bytes_available_mock,
      .read_byte = read_byte_mock,
      .send_byte = send_byte_mock
    },
  }), SDS011_OK);

  assert_int_equal(sds011_get_reporting_mode(NULL, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  assert_int_equal(sds011_get_reporting_mode(&sds011, 0xFFFF, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

static void test_set_sleep_on(void **state) {
  (void) state; /* unused */

  sds011_t sds011;

  assert_int_equal(sds011_init(&sds011, &(sds011_init_t) {
    .millis = millis_mock,
    .serial = {
      .bytes_available = bytes_available_mock,
      .read_byte = read_byte_mock,
      .send_byte = send_byte_mock
    },
  }), SDS011_OK);

  assert_int_equal(sds011_set_sleep_on(NULL, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  assert_int_equal(sds011_set_sleep_on(&sds011, 0xA160, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x08, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

static void test_set_sleep_off(void **state) {
  (void) state; /* unused */

  sds011_t sds011;

  assert_int_equal(sds011_init(&sds011, &(sds011_init_t) {
    .millis = millis_mock,
    .serial = {
      .bytes_available = bytes_available_mock,
      .read_byte = read_byte_mock,
      .send_byte = send_byte_mock
    },
  }), SDS011_OK);

  assert_int_equal(sds011_set_sleep_off(NULL, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  assert_int_equal(sds011_set_sleep_off(&sds011, 0xA160, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x06, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x09, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

static void test_get_sleep(void **state) {
  (void) state; /* unused */

  sds011_t sds011;

  assert_int_equal(sds011_init(&sds011, &(sds011_init_t) {
    .millis = millis_mock,
    .serial = {
      .bytes_available = bytes_available_mock,
      .read_byte = read_byte_mock,
      .send_byte = send_byte_mock
    },
  }), SDS011_OK);

  assert_int_equal(sds011_get_sleep(NULL, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  assert_int_equal(sds011_get_sleep(&sds011, 0xA160, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x07, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

static void test_set_op_mode_continous(void **state) {
  (void) state; /* unused */

  sds011_t sds011;

  assert_int_equal(sds011_init(&sds011, &(sds011_init_t) {
    .millis = millis_mock,
    .serial = {
      .bytes_available = bytes_available_mock,
      .read_byte = read_byte_mock,
      .send_byte = send_byte_mock
    },
  }), SDS011_OK);

  assert_int_equal(sds011_set_op_mode_continous(NULL, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  assert_int_equal(sds011_set_op_mode_continous(&sds011, 0xA160, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x0A, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

static void test_set_op_mode_periodic(void **state) {
  (void) state; /* unused */

  sds011_t sds011;

  assert_int_equal(sds011_init(&sds011, &(sds011_init_t) {
    .millis = millis_mock,
    .serial = {
      .bytes_available = bytes_available_mock,
      .read_byte = read_byte_mock,
      .send_byte = send_byte_mock
    },
  }), SDS011_OK);

  assert_int_equal(sds011_set_op_mode_periodic(NULL, 0, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  assert_int_equal(sds011_set_op_mode_periodic(&sds011, 0xA160, 1, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x08, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x0B, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

static void test_get_op_mode(void **state) {
  (void) state; /* unused */

  sds011_t sds011;

  assert_int_equal(sds011_init(&sds011, &(sds011_init_t) {
    .millis = millis_mock,
    .serial = {
      .bytes_available = bytes_available_mock,
      .read_byte = read_byte_mock,
      .send_byte = send_byte_mock
    },
  }), SDS011_OK);

  assert_int_equal(sds011_get_op_mode(NULL, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  assert_int_equal(sds011_get_op_mode(&sds011, 0xA160, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x09, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

static void test_get_fw_ver(void **state) {
  (void) state; /* unused */

  sds011_t sds011;

  assert_int_equal(sds011_init(&sds011, &(sds011_init_t) {
    .millis = millis_mock,
    .serial = {
      .bytes_available = bytes_available_mock,
      .read_byte = read_byte_mock,
      .send_byte = send_byte_mock
    },
  }), SDS011_OK);

  assert_int_equal(sds011_get_fw_ver(NULL, 0, (sds011_cb_t){NULL, NULL}), SDS011_ERR_INVALID_PARAM);

  send_byte_iter = 0;
  assert_int_equal(sds011_get_fw_ver(&sds011, 0xA160, (sds011_cb_t){NULL, NULL}), SDS011_OK);
  uint8_t ref[] = {
    0xAA, 0xB4, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA1, 0x60, 0x08, 0xAB
  };
  assert_memory_equal(send_byte_buffer, ref, SDS011_QUERY_PACKET_SIZE);
}

int tests_sds011(void) {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_init),
    cmocka_unit_test(test_query_data),
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
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
