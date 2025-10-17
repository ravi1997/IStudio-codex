#include <cstring>
#include <iostream>

#include "support/version.h"

namespace {

constexpr const char* usage = R"(IStudio Compiler

Usage:
  istudio --version     Print the compiler version
  istudio <command>     Placeholder for future commands
)";

}  // namespace

int main(int argc, char* argv[]) {
  if (argc > 1) {
    if (std::strcmp(argv[1], "--version") == 0 || std::strcmp(argv[1], "-V") == 0) {
      std::cout << "IStudio " << istudio::support::current_version() << '\n';
      return 0;
    }

    std::cout << "Unrecognized command '" << argv[1] << "'\n\n" << usage;
    return 1;
  }

  std::cout << usage;
  return 0;
}
