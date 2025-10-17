#include "support/diagnostics.h"

namespace istudio::support {

void DiagnosticReporter::report(DiagCode code, std::string message, Span span) {
  diagnostics_.push_back(Diagnostic{.code = code, .message = std::move(message), .span = span, .notes = {}});
}

std::string_view to_string(DiagCode code) {
  switch (code) {
    case DiagCode::GenericNote:
      return "GenericNote";
    case DiagCode::LexUnknownToken:
      return "LexUnknownToken";
  }
  return "Unknown";
}

}  // namespace istudio::support
