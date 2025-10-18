set(ISTUDIO_CORE_SOURCES
  backends/cpp/cpp_backend.cpp
  front/token.cpp
  front/lexer.cpp
  front/parser.cpp
  front/ast.cpp
  front/ast_dump.cpp
  sem/context.cpp
  sem/analyzer.cpp
  ir/module.cpp
  ir/printer.cpp
  ir/lowering.cpp
  opt/pass_manager.cpp
  opt/constant_folding.cpp
  lsp/message_io.cpp
  lsp/server.cpp
  support/diagnostics.cpp
  support/version.cpp
  plugins/registry.cpp
)

add_library(istudio_core STATIC ${ISTUDIO_CORE_SOURCES})
target_include_directories(istudio_core PUBLIC ${CMAKE_CURRENT_LIST_DIR})
istudio_enable_warnings(istudio_core)

add_executable(istudio cli/main.cpp)
target_link_libraries(istudio PRIVATE istudio_core)
target_include_directories(istudio PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_compile_definitions(istudio PRIVATE ISTUDIO_VERSION="${PROJECT_VERSION}")
istudio_enable_warnings(istudio)
