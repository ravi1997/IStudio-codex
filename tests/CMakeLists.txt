set(ISTUDIO_TEST_SOURCES
  front/test_lexer.cpp
)

add_executable(istudio_tests ${ISTUDIO_TEST_SOURCES})
target_link_libraries(istudio_tests PRIVATE istudio_core)
target_include_directories(istudio_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
istudio_enable_warnings(istudio_tests)

add_test(NAME istudio_tests COMMAND istudio_tests)
