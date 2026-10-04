# ctest suite for mlx-data-c: every test asserts outcomes and exits
# non-zero on mismatch. Fixtures live in tests/fixtures.

set(MLXD_TEST_FIXTURES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fixtures)

function(mlxd_add_test name)
  add_executable(${name} ${name}.c)
  target_link_libraries(${name} PRIVATE mlxdc)
  target_compile_definitions(
    ${name} PRIVATE MLXD_FIXTURES_DIR="${MLXD_TEST_FIXTURES_DIR}")
  target_compile_options(${name} PRIVATE -Wall -Wextra -Werror)
  add_test(NAME ${name} COMMAND ${name})
endfunction()

mlxd_add_test(test_array)
mlxd_add_test(test_sample)
mlxd_add_test(test_buffer)
mlxd_add_test(test_stream)
mlxd_add_test(test_ops)
mlxd_add_test(test_tokenizer)
mlxd_add_test(test_misc)
