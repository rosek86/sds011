cmake_minimum_required(VERSION 3.1)
project(tests)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -std=c11 -g -O0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")

find_library(CMOCKA_LIB cmocka)
find_path(CMOCKA_INC cmocka.h)

include_directories(${CMOCKA_INC})

add_executable(tests
  ../src/sds011_parser.c    ./tests_parser.c
  ../src/sds011_builder.c   ./tests_builder.c
  ../src/sds011_validator.c ./tests_validator.c
  ../src/sds011_fifo.c      ./tests_fifo.c
  ../src/sds011.c           ./tests_sds011.c
  ./tests.c
)

# enable_testing()
# add_test(tests ./tests)

# add_custom_command(TARGET tests
#   POST_BUILD
#   COMMAND lcov --zerocounters --directory . || true
#   COMMAND ./tests
#   COMMAND lcov --capture --directory . --output-file coverage.info --rc lcov_branch_coverage=1
#   COMMAND genhtml coverage.info --output-directory out --rc lcov_branch_coverage=1
#   COMMAND lcov -v
# )

add_custom_target(test
  COMMAND lcov --zerocounters --directory . || true
  COMMAND ./tests
)

add_custom_target(coverage
  COMMAND lcov --capture --directory . --output-file coverage.info --rc lcov_branch_coverage=1
  COMMAND genhtml coverage.info --output-directory out --rc lcov_branch_coverage=1
  COMMAND lcov -v
)

target_link_libraries(tests ${CMOCKA_LIB})
