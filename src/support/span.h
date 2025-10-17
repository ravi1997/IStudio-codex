#pragma once

#include <cstddef>
#include <ostream>

namespace istudio::support {

struct Span {
  std::size_t start{0};
  std::size_t end{0};

  [[nodiscard]] constexpr std::size_t length() const noexcept {
    return end >= start ? end - start : 0;
  }
};

inline std::ostream& operator<<(std::ostream& os, const Span& span) {
  os << '[' << span.start << ", " << span.end << ')';
  return os;
}

}  // namespace istudio::support
