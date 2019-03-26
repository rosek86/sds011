cmake_minimum_required(VERSION 3.14)

find_package(CMOCKA REQUIRED)

function(create_test)
  cmake_parse_arguments(CREATE_TEST "" "NAME;FIXTURE" "FILES" ${ARGN})

  add_executable(${CREATE_TEST_NAME} ${CREATE_TEST_FILES})

  set_property(TARGET ${CREATE_TEST_NAME} PROPERTY C_STANDARD 11)

  target_include_directories(${CREATE_TEST_NAME} PRIVATE ${CMOCKA_INCLUDE_DIR})

  target_compile_options(${CREATE_TEST_NAME} PRIVATE -Wall -Wextra -pedantic)
  target_compile_options(${CREATE_TEST_NAME} PRIVATE -g -O0)

  target_compile_options(${CREATE_TEST_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
  target_link_options(${CREATE_TEST_NAME} PRIVATE -fprofile-arcs -ftest-coverage)

  target_link_libraries(${CREATE_TEST_NAME} ${CMOCKA_LIBRARIES})

  add_test(NAME ${CREATE_TEST_NAME} COMMAND ./${CREATE_TEST_NAME})
  set_tests_properties(${CREATE_TEST_NAME} PROPERTIES FIXTURES_REQUIRED ${CREATE_TEST_FIXTURE})
endfunction()

add_custom_target(coverage-zerocounters
  COMMAND lcov --zerocounters --directory .
)

add_custom_target(coverage
  COMMAND lcov --capture --directory . --output-file coverage.info --rc lcov_branch_coverage=1
  COMMAND genhtml coverage.info --output-directory coverage --rc lcov_branch_coverage=1
  COMMAND lcov -v
)

add_test(NAME setup COMMAND cmake --build .. --target coverage-zerocounters)
set_tests_properties(setup PROPERTIES FIXTURES_SETUP tests-fixture)

create_test(NAME test_builder   FIXTURE tests-fixture FILES ../src/sds011_builder.c   ./tests_builder.c)
create_test(NAME test_parser    FIXTURE tests-fixture FILES ../src/sds011_parser.c    ./tests_parser.c)
create_test(NAME test_validator FIXTURE tests-fixture FILES ../src/sds011_validator.c ./tests_validator.c)
create_test(NAME test_fifo      FIXTURE tests-fixture FILES ../src/sds011_fifo.c      ./tests_fifo.c)
create_test(NAME test_sds011    FIXTURE tests-fixture FILES
  ../src/sds011_builder.c
  ../src/sds011_parser.c
  ../src/sds011_validator.c
  ../src/sds011_fifo.c 
  ../src/sds011.c ./tests_sds011.c
)

add_test(NAME cleanup COMMAND echo "cleanup")
set_tests_properties(cleanup PROPERTIES FIXTURES_CLEANUP tests-fixture)
