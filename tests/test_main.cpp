#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>

void run_lexer_tests();
void run_parser_tests();
void run_ast_dump_tests();
void run_semantic_tests();
void run_ir_tests();
void run_ir_lowering_tests();
void run_cpp_backend_tests();
void run_lsp_tests();

int main() {
  try {
    run_lexer_tests();
    run_parser_tests();
    run_ast_dump_tests();
    run_semantic_tests();
    run_ir_tests();
    run_ir_lowering_tests();
    run_cpp_backend_tests();
    run_lsp_tests();
  } catch (const std::exception& ex) {
    std::cerr << "[tests] " << ex.what() << '\n';
    return EXIT_FAILURE;
  }

  std::cout << "All tests completed successfully\n";
  return EXIT_SUCCESS;
}
