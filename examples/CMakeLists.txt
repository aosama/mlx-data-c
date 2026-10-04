# Pure-C examples: compiled as C on purpose — if C++ leaks into the public
# headers, these translation units stop compiling.

set(MLXD_C_EXAMPLES example_buffer example_tokenize example_stream)

foreach(example ${MLXD_C_EXAMPLES})
  add_executable(${example} ${CMAKE_CURRENT_LIST_DIR}/${example}.c)
  target_link_libraries(${example} PRIVATE mlxdc)
  target_compile_options(${example} PRIVATE -Wall -Wextra -Werror)
  set_target_properties(${example} PROPERTIES C_STANDARD 11)
endforeach()
