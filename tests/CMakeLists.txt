cmake_minimum_required(VERSION 3.14)
project(tests)

find_package(CMOCKA REQUIRED)

add_executable(tests
  ../src/sds011_parser.c    ./tests_parser.c
  ../src/sds011_builder.c   ./tests_builder.c
  ../src/sds011_validator.c ./tests_validator.c
  ../src/sds011_fifo.c      ./tests_fifo.c
  ../src/sds011.c           ./tests_sds011.c
  ./tests.c
)

set_property(TARGET tests PROPERTY C_STANDARD 11)

target_include_directories(tests PRIVATE ${CMOCKA_INCLUDE_DIR})

target_compile_options(tests PRIVATE -Wall -Wextra -pedantic)
target_compile_options(tests PRIVATE -g -O0)

target_compile_options(tests PRIVATE -fprofile-arcs -ftest-coverage)
target_link_options(tests PRIVATE -fprofile-arcs -ftest-coverage)

add_test(NAME setup COMMAND lcov --zerocounters --directory .)
set_tests_properties(setup PROPERTIES FIXTURES_SETUP my-fixture)

add_test(NAME test_all COMMAND ./tests)
set_tests_properties(test_all PROPERTIES FIXTURES_REQUIRED my-fixture)

add_test(NAME cleanup COMMAND echo "cleanup")
set_tests_properties(cleanup PROPERTIES FIXTURES_CLEANUP my-fixture)

# add_custom_command(TARGET tests
#   POST_BUILD
#   COMMAND lcov --zerocounters --directory . || true
#   COMMAND ./tests
#   COMMAND lcov --capture --directory . --output-file coverage.info --rc lcov_branch_coverage=1
#   COMMAND genhtml coverage.info --output-directory out --rc lcov_branch_coverage=1
#   COMMAND lcov -v
# )

add_custom_target(coverage
  COMMAND lcov --capture --directory . --output-file coverage.info --rc lcov_branch_coverage=1
  COMMAND genhtml coverage.info --output-directory coverage --rc lcov_branch_coverage=1
  COMMAND lcov -v
)

target_link_libraries(tests ${CMOCKA_LIBRARIES})
