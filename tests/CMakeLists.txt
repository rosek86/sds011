cmake_minimum_required(VERSION 2.8)
project(tests)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -std=c11 -g -O0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLGGS} -fprofile-arcs -ftest-coverage -fno-elide-constructors")

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

add_custom_target(
  clear-coverage
  COMMAND find . -name "*gcd[ao]" -exec rm -f {} +
)

add_dependencies(tests clear-coverage)
target_link_libraries(tests ${CMOCKA_LIB})
