set(ISTUDIO_CORE_SOURCES
  front/token.cpp
  front/lexer.cpp
  front/ast.cpp
  sem/context.cpp
  ir/module.cpp
  opt/pass_manager.cpp
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
