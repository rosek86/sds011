cmake_minimum_required(VERSION 3.14)
project(sds011)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(./examples)

enable_testing()
add_subdirectory(./tests)

add_custom_target(lint COMMAND cppcheck
  --enable=warning
  --project=compile_commands.json
  --quiet
)
