#include "mathbasics.hpp"

namespace istudio::generated {

std::int32_t add(std::int32_t a, std::int32_t b) {
  return a + b;
}

std::int32_t triple(std::int32_t value) {
  auto doubled = add(value, value);
  return doubled + value;
}

}  // namespace istudio::generated
