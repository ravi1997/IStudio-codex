#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "support/span.h"

namespace istudio::support {

enum class DiagCode {
  GenericNote = 0,
  LexUnknownToken = 1000,
};

struct Diagnostic {
  DiagCode code{DiagCode::GenericNote};
  std::string message{};
  Span span{};
  std::vector<std::string> notes{};
};

class DiagnosticReporter {
 public:
  void report(DiagCode code, std::string message, Span span);
  [[nodiscard]] const std::vector<Diagnostic>& diagnostics() const noexcept { return diagnostics_; }

 private:
  std::vector<Diagnostic> diagnostics_{};
};

std::string_view to_string(DiagCode code);

}  // namespace istudio::support
