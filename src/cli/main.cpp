#include <iostream>
#include <string_view>

#include "lsp/server.h"
#include "support/version.h"

namespace {

constexpr const char* usage = R"(IStudio Compiler

Usage:
  istudio --version            Print the compiler version
  istudio --help               Print this message
  istudio lsp                  Start the language server on stdio
  istudio <command> [args...]  Placeholder for future commands
)";

}  // namespace

int main(int argc, char* argv[]) {
  if (argc <= 1) {
    std::cout << usage;
    return 0;
  }

  std::string_view command{argv[1]};

  if (command == "--version" || command == "-V") {
    std::cout << "IStudio " << istudio::support::current_version() << '\n';
    return 0;
  }

  if (command == "--help" || command == "-h") {
    std::cout << usage;
    return 0;
  }

  if (command == "lsp") {
    istudio::lsp::Server server;
    return server.run(std::cin, std::cout);
  }

  std::cout << "Unrecognized command '" << command << "'\n\n" << usage;
  return 1;
}
