#include "stringformatting.hpp"

namespace istudio::generated {

std::string greet(std::string name) {
  return std::string("Hello, ") + name;
}

std::string decorated(std::string name, std::string punctuation) {
  auto base = greet(name);
  return base + punctuation;
}

}  // namespace istudio::generated
