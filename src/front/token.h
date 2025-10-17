#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "support/span.h"

namespace istudio::front {

enum class TokenKind {
  Identifier,
  Number,
  StringLiteral,
  Keyword,
  Symbol,
  EndOfFile,
  Unknown,
};

enum class TriviaKind {
  Whitespace,
  Comment,
};

struct Trivia {
  TriviaKind kind{TriviaKind::Whitespace};
  std::string text{};
  support::Span span{};
};

struct Token {
  TokenKind kind{TokenKind::Unknown};
  std::string lexeme{};
  support::Span span{};
  std::vector<Trivia> leading_trivia{};
  std::vector<Trivia> trailing_trivia{};
};

struct LexerConfig {
  bool capture_whitespace{false};
  bool capture_comments{true};
};

struct TokenStream {
  std::vector<Token> tokens{};

  [[nodiscard]] auto begin() noexcept { return tokens.begin(); }
  [[nodiscard]] auto end() noexcept { return tokens.end(); }
  [[nodiscard]] auto begin() const noexcept { return tokens.begin(); }
  [[nodiscard]] auto end() const noexcept { return tokens.end(); }
  [[nodiscard]] auto size() const noexcept { return tokens.size(); }
  [[nodiscard]] bool empty() const noexcept { return tokens.empty(); }
  [[nodiscard]] const Token& back() const { return tokens.back(); }
};

std::string_view to_string(TokenKind kind);

}  // namespace istudio::front
