#pragma once

#include <string_view>

#include "front/token.h"

namespace istudio::front {

class Lexer {
 public:
  explicit Lexer(std::string_view source, LexerConfig config = {});

  [[nodiscard]] TokenStream lex();

 private:
  [[nodiscard]] Token read_identifier();
  [[nodiscard]] Token read_number();
  [[nodiscard]] Token read_string();
  [[nodiscard]] Token read_symbol();
  [[nodiscard]] Trivia make_trivia(TriviaKind kind, std::size_t start, std::size_t end) const;
  void skip_whitespace();
  void capture_trivia(TriviaKind kind, std::size_t start, std::size_t end, std::vector<Trivia>& target);

  std::string_view source_;
  LexerConfig config_{};
  std::size_t position_{0};
  std::vector<Trivia> pending_leading_{};
};

TokenStream lex(std::string_view source, const LexerConfig& config = {});

}  // namespace istudio::front
