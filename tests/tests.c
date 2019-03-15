#include "tests.h"

int main(void) {
  (void)tests_parser();
  (void)tests_builder();
  (void)tests_fifo();
  (void)tests_sds011();
  (void)tests_validator();
  return 0;
}
