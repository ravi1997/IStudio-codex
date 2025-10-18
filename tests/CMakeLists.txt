set(ISTUDIO_TEST_SOURCES
  backends/test_cpp_backend.cpp
  front/test_lexer.cpp
  front/test_parser.cpp
  front/test_ast_dump.cpp
  sem/test_semantic.cpp
  ir/test_ir.cpp
  ir/test_lowering.cpp
  lsp/test_lsp.cpp
  test_main.cpp
)

add_executable(istudio_tests ${ISTUDIO_TEST_SOURCES})
target_link_libraries(istudio_tests PRIVATE istudio_core)
target_include_directories(istudio_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
istudio_enable_warnings(istudio_tests)

add_test(NAME istudio_tests COMMAND istudio_tests)
