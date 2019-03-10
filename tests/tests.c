#include "tests.h"

int main(void) {
  tests_parser();
  tests_builder();
  tests_fifo();
  tests_sds011();
  tests_validator();
  return 0;
}
