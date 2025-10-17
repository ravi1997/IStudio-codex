#include "front/token.h"

namespace istudio::front {

std::string_view to_string(TokenKind kind) {
  switch (kind) {
    case TokenKind::Identifier:
      return "Identifier";
    case TokenKind::Number:
      return "Number";
    case TokenKind::StringLiteral:
      return "StringLiteral";
    case TokenKind::Keyword:
      return "Keyword";
    case TokenKind::Symbol:
      return "Symbol";
    case TokenKind::EndOfFile:
      return "EndOfFile";
    case TokenKind::Unknown:
      return "Unknown";
  }
  return "Unknown";
}

}  // namespace istudio::front
