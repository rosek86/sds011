/*lint -e818*/
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>

#include "../src/sds011_validator.h"

static void test_validate(void **state) {
  (void) state; /* unused */

  // invalid req, res
  assert_int_equal(sds011_validator_validate(NULL, NULL), false);
  assert_int_equal(sds011_validator_validate(&(sds011_msg_t) {
    .dev_id = 0,
  }, NULL), false);

  // req.type != res.type
  assert_int_equal(sds011_validator_validate(&(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_REP_MODE,
  }, &(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_DATA,
  }), false);

  // req.op ! = res.op
  assert_int_equal(sds011_validator_validate(&(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_REP_MODE,
    .op   = SDS011_MSG_OP_GET,
  }, &(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_REP_MODE,
    .op   = SDS011_MSG_OP_SET,
  }), false);

  // req->op == SDS011_MSG_OP_GE
  assert_int_equal(sds011_validator_validate(&(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_REP_MODE,
    .op   = SDS011_MSG_OP_GET,
  }, &(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_REP_MODE,
    .op   = SDS011_MSG_OP_GET,
  }), true);
}

static void test_no_validator_function(void **state) {
  (void) state; /* unused */

  assert_int_equal(sds011_validator_validate(&(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_REP_MODE,
    .op   = SDS011_MSG_OP_SET,
  }, &(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_REP_MODE,
    .op   = SDS011_MSG_OP_SET,
  }), true);
}

static void test_validate_rep_mode(void **state) {
  (void) state; /* unused */

  // success
  assert_int_equal(sds011_validator_validate(&(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_REP_MODE,
    .op   = SDS011_MSG_OP_SET,
    .data = {
      .rep_mode = SDS011_REP_MODE_ACTIVE,
    }
  }, &(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_REP_MODE,
    .op   = SDS011_MSG_OP_SET,
    .data = {
      .rep_mode = SDS011_REP_MODE_ACTIVE,
    }
  }), true);

  // failure
  assert_int_equal(sds011_validator_validate(&(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_REP_MODE,
    .op   = SDS011_MSG_OP_SET,
    .data = {
      .rep_mode = SDS011_REP_MODE_ACTIVE,
    }
  }, &(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_REP_MODE,
    .op   = SDS011_MSG_OP_SET,
    .data = {
      .rep_mode = SDS011_REP_MODE_QUERY,
    }
  }), false);
}

static void test_validate_dev_id(void **state) {
  (void) state; /* unused */

  // success
  assert_int_equal(sds011_validator_validate(&(sds011_msg_t) {
    .type   = SDS011_MSG_TYPE_DEV_ID,
    .op     = SDS011_MSG_OP_SET,
    .dev_id = 0xFFFF,
    .data = {
      .new_dev_id = 0x1234
    }
  }, &(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_DEV_ID,
    .op   = SDS011_MSG_OP_SET,
    .dev_id = 0x1234,
  }), true);

  // failure
  assert_int_equal(sds011_validator_validate(&(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_DEV_ID,
    .op   = SDS011_MSG_OP_SET,
    .dev_id = 0xFFFF,
    .data = {
      .new_dev_id = 0x1234
    }
  }, &(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_DEV_ID,
    .op   = SDS011_MSG_OP_SET,
    .dev_id = 0x4321,
  }), false);
}

static void test_validate_sleep(void **state) {
  (void) state; /* unused */

  // success
  assert_int_equal(sds011_validator_validate(&(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_SLEEP,
    .op   = SDS011_MSG_OP_SET,
    .data = {
      .sleep = SDS011_SLEEP_ON,
    }
  }, &(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_SLEEP,
    .op   = SDS011_MSG_OP_SET,
    .data = {
      .sleep = SDS011_SLEEP_ON,
    }
  }), true);

  // failure
  assert_int_equal(sds011_validator_validate(&(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_SLEEP,
    .op   = SDS011_MSG_OP_SET,
    .data = {
      .sleep = SDS011_SLEEP_ON,
    }
  }, &(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_SLEEP,
    .op   = SDS011_MSG_OP_SET,
    .data = {
      .sleep = SDS011_SLEEP_OFF,
    }
  }), false);
}


static void test_validate_op_mode(void **state) {
  (void) state; /* unused */

  // success
  assert_int_equal(sds011_validator_validate(&(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_OP_MODE,
    .op   = SDS011_MSG_OP_SET,
    .data = {
      .op_mode = {
        .mode = SDS011_OP_MODE_CONTINOUS,
        .interval = 0,
      },
    }
  }, &(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_OP_MODE,
    .op   = SDS011_MSG_OP_SET,
    .data = {
      .op_mode = {
        .mode = SDS011_OP_MODE_CONTINOUS,
        .interval = 0,
      },
    }
  }), true);

  // failure
  assert_int_equal(sds011_validator_validate(&(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_OP_MODE,
    .op   = SDS011_MSG_OP_SET,
    .data = {
      .op_mode = {
        .mode = SDS011_OP_MODE_CONTINOUS,
        .interval = 0,
      }
    }
  }, &(sds011_msg_t) {
    .type = SDS011_MSG_TYPE_OP_MODE,
    .op   = SDS011_MSG_OP_SET,
    .data = {
      .op_mode = {
        .mode = SDS011_OP_MODE_INTERVAL,
        .interval = 0,
      }
    }
  }), false);
}

int main(void) {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_validate),
    cmocka_unit_test(test_no_validator_function),
    cmocka_unit_test(test_validate_rep_mode),
    cmocka_unit_test(test_validate_dev_id),
    cmocka_unit_test(test_validate_sleep),
    cmocka_unit_test(test_validate_op_mode)
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
