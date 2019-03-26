cmake_minimum_required(VERSION 3.14)
project(sds011)

add_subdirectory(./examples)

enable_testing()
add_subdirectory(./tests)

add_custom_target(lint COMMAND cppcheck --enable=all
  ${CMAKE_SOURCE_DIR}/tests/
  ${CMAKE_SOURCE_DIR}/src/
)
