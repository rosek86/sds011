/*lint -e818*/
#include "tests.h"
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "../src/sds011_fifo.h"

static void test_init_no_segfault(void **state) {
  (void)state;
  assert_false(sds011_fifo_init(NULL, 0, NULL, 0));
}

static void test_init_params(void **state) {
  (void) state;

  sds011_fifo_t f, *fifo = &f;
  uint8_t mem[2];

  assert_false(sds011_fifo_init(NULL, 0, NULL, 0));
  assert_false(sds011_fifo_init(fifo, 0, NULL, 0));
  assert_false(sds011_fifo_init(fifo, 1, NULL, 0));
  assert_false(sds011_fifo_init(fifo, 0, mem,  0));
  assert_false(sds011_fifo_init(fifo, 1, mem,  0));
  assert_true (sds011_fifo_init(fifo, 1, mem,  1));
  assert_false(sds011_fifo_init(fifo, 2, mem,  1));
  assert_true (sds011_fifo_init(fifo, 2, mem,  2));

  assert_int_equal(fifo->elsize, 2);
  assert_int_equal(fifo->mem, mem);
  assert_int_equal(fifo->size, 2);
  assert_int_equal(fifo->beg, 0);
  assert_int_equal(fifo->end, 0);
}

static void test_init_count_calcs(void **state) {
  (void) state;

  sds011_fifo_t f, *fifo = &f;
  uint8_t mem[4];

  assert_true(sds011_fifo_init(fifo, 2, mem, 2));
  assert_int_equal(fifo->count, 1);

  assert_true(sds011_fifo_init(fifo, 2, mem, 3));
  assert_int_equal(fifo->count, 1);

  assert_true(sds011_fifo_init(fifo, 2, mem, 4));
  assert_int_equal(fifo->count, 2);
}

static void test_push_params(void **state) {
  (void)state;
  sds011_fifo_t f, *fifo = &f;
  uint8_t mem[2*sizeof(uint16_t)];
  uint16_t v = 65;

  assert_true(sds011_fifo_init(fifo, sizeof(uint16_t), mem, sizeof(mem)));
  assert_false(sds011_fifo_push(NULL, NULL));
  assert_false(sds011_fifo_push(fifo, NULL));
  assert_true(sds011_fifo_push(fifo, &v));
}

static void test_pop_params(void **state) {
  (void)state;
  sds011_fifo_t f, *fifo = &f;
  uint8_t mem[2*sizeof(uint16_t)];
  uint16_t v = 65;

  assert_true (sds011_fifo_init(fifo, sizeof(uint16_t), mem, sizeof(mem)));
  assert_true (sds011_fifo_push(fifo, &v));

  assert_false(sds011_fifo_pop(NULL, NULL));
  assert_false(sds011_fifo_pop(fifo, NULL));
  assert_true (sds011_fifo_pop(fifo, &v));
}

static void test_pop_but_empty(void **state) {
  (void) state;

  sds011_fifo_t f, *fifo = &f;
  uint16_t v1, v2;

  assert_true(sds011_fifo_init(fifo, 2, &v1,  2));
  assert_false(sds011_fifo_pop(fifo, &v2));
}

static void test_push_but_full(void **state) {
  (void) state;

  sds011_fifo_t f, *fifo = &f;
  uint16_t v1, v2 = 5;
  uint8_t mem[2*sizeof(uint16_t)];

  assert_true(sds011_fifo_init(fifo, sizeof(uint16_t), &v1,  sizeof(uint16_t)));
  assert_false(sds011_fifo_push(fifo, &v2)); 

  assert_true (sds011_fifo_init(fifo, sizeof(uint16_t), mem,  sizeof(mem)));
  assert_true (sds011_fifo_push(fifo, &v2));
  assert_false(sds011_fifo_push(fifo, &v2));
}

static void test_push_pop(void **state) {
  (void) state;

  sds011_fifo_t f, *fifo = &f;
  uint16_t v1 = 0, v2 = 0x1234;
  uint8_t mem[2*sizeof(uint16_t)];

  assert_true (sds011_fifo_init(fifo, sizeof(uint16_t), mem,  sizeof(mem)));

  assert_true (sds011_fifo_push(fifo, &v2));
  assert_int_equal(fifo->beg, 0);
  assert_int_equal(fifo->end, 1);

  assert_true (sds011_fifo_pop (fifo, &v1));
  assert_int_equal(fifo->beg, 1);
  assert_int_equal(fifo->end, 1);

  assert_int_equal(v1, v2);
}

static void test_circular_buffer(void **state) {
  (void) state;

  sds011_fifo_t f, *fifo = &f;
  uint16_t v1 = 0, v2 = 0x1234;
  uint8_t mem[2*sizeof(uint16_t)];

  assert_true (sds011_fifo_init(fifo, sizeof(uint16_t), mem,  sizeof(mem)));

  assert_int_equal(fifo->beg, 0);
  assert_int_equal(fifo->end, 0);
  assert_true (sds011_fifo_push(fifo, &v2));
  assert_int_equal(fifo->beg, 0);
  assert_int_equal(fifo->end, 1);
  assert_true (sds011_fifo_pop (fifo, &v1));
  assert_int_equal(fifo->beg, 1);
  assert_int_equal(fifo->end, 1);
  assert_true (sds011_fifo_push(fifo, &v2));
  assert_int_equal(fifo->beg, 1);
  assert_int_equal(fifo->end, 0);
  assert_true (sds011_fifo_pop (fifo, &v1));
  assert_int_equal(fifo->beg, 0);
  assert_int_equal(fifo->end, 0);

  assert_int_equal(v1, v2);
}

int tests_fifo(void) {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_init_no_segfault),
    cmocka_unit_test(test_init_params),
    cmocka_unit_test(test_init_count_calcs),
    cmocka_unit_test(test_push_params),
    cmocka_unit_test(test_pop_params),
    cmocka_unit_test(test_pop_but_empty),
    cmocka_unit_test(test_push_but_full),
    cmocka_unit_test(test_push_pop),
    cmocka_unit_test(test_circular_buffer),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
