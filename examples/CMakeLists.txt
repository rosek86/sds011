cmake_minimum_required(VERSION 2.8)
project(examples)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c11")

add_executable(parser_example
  ../src/sds011_parser.c
  ./parser_example.c
)

add_executable(example
  ../src/sds011_parser.c
  ../src/sds011_builder.c
  ../src/sds011.c
  ./example.c
)
