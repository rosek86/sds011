cmake_minimum_required(VERSION 2.8)
project(tests)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c11")

find_library(CMOCKA_LIB cmocka)

include_directories(
  /usr/local/include/
)

add_executable(tests
  ../src/sds011_parser.c    ./tests_parser.c
  ../src/sds011_builder.c   ./tests_builder.c
  ./tests.c
)

target_link_libraries(tests ${CMOCKA_LIB})
