cmake_minimum_required(VERSION 2.8)
project(parse_sample)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_executable(parse_sample
  ../src/sds011_parser.c
  ./parse_sample.c
)

target_link_libraries(parse_sample)
