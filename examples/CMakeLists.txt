cmake_minimum_required(VERSION 3.14)
project(examples)

add_executable(parser_example
  ../src/sds011_parser.c
  ./parser_example.c
)

set_property(TARGET parser_example PROPERTY C_STANDARD 11)

target_compile_options(parser_example PRIVATE -Wall -Wextra -pedantic)
target_compile_options(parser_example PRIVATE -g -O0)

target_compile_options(parser_example PRIVATE -fprofile-arcs -ftest-coverage)
target_link_options(parser_example PRIVATE -fprofile-arcs -ftest-coverage)

add_executable(example
  ../src/sds011_fifo.c
  ../src/sds011_parser.c
  ../src/sds011_builder.c
  ../src/sds011_validator.c
  ../src/sds011.c
  ./example.c
)

set_property(TARGET example PROPERTY C_STANDARD 11)

target_compile_options(example PRIVATE -Wall -Wextra -pedantic)
target_compile_options(example PRIVATE -g -O0)

target_compile_options(example PRIVATE -fprofile-arcs -ftest-coverage)
target_link_options(example PRIVATE -fprofile-arcs -ftest-coverage)
