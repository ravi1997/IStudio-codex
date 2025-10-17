#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>

void run_lexer_tests();
void run_parser_tests();

int main() {
  try {
    run_lexer_tests();
    run_parser_tests();
  } catch (const std::exception& ex) {
    std::cerr << "[tests] " << ex.what() << '\n';
    return EXIT_FAILURE;
  }

  std::cout << "All tests completed successfully\n";
  return EXIT_SUCCESS;
}
