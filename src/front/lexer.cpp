#include "front/lexer.h"

#include <array>
#include <cctype>
#include <utility>

namespace istudio::front {
namespace {

bool is_identifier_start(char ch) {
  return std::isalpha(static_cast<unsigned char>(ch)) || ch == '_';
}

bool is_identifier_continue(char ch) {
  return is_identifier_start(ch) || std::isdigit(static_cast<unsigned char>(ch));
}

bool is_digit(char ch) {
  return std::isdigit(static_cast<unsigned char>(ch));
}

bool is_keyword(std::string_view word) {
  constexpr std::array<std::string_view, 9> keywords{
      "module", "fn",   "pub",   "let", "mut", "struct", "enum", "ct", "return"};
  for (auto kw : keywords) {
    if (kw == word) {
      return true;
    }
  }
  return false;
}

bool is_compound_symbol(std::string_view symbol) {
  constexpr std::array<std::string_view, 20> compound{
      "==", "!=", "<=", ">=", "&&", "||", "::", "->", "=>", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<",
      ">>", ">>="};
  for (auto sym : compound) {
    if (sym == symbol) {
      return true;
    }
  }
  return false;
}

}  // namespace

Lexer::Lexer(std::string_view source, LexerConfig config)
    : source_(source), config_(config) {}

TokenStream Lexer::lex() {
  TokenStream stream{};
  while (position_ < source_.size()) {
    skip_whitespace();
    if (position_ >= source_.size()) {
      break;
    }

    if (source_[position_] == '/' && position_ + 1 < source_.size() && source_[position_ + 1] == '/') {
      const auto start = position_;
      position_ += 2;
      while (position_ < source_.size() && source_[position_] != '\n') {
        ++position_;
      }
      capture_trivia(TriviaKind::Comment, start, position_, pending_leading_);
      continue;
    }

    Token token{};
    if (is_identifier_start(source_[position_])) {
      token = read_identifier();
    } else if (is_digit(source_[position_])) {
      token = read_number();
    } else if (source_[position_] == '"') {
      token = read_string();
    } else {
      token = read_symbol();
    }
    stream.tokens.push_back(std::move(token));
  }

  Token eof{};
  eof.kind = TokenKind::EndOfFile;
  eof.lexeme = "";
  eof.span = {source_.size(), source_.size()};
  eof.leading_trivia = std::exchange(pending_leading_, {});
  stream.tokens.push_back(std::move(eof));
  return stream;
}

Token Lexer::read_identifier() {
  const auto start = position_;
  ++position_;
  while (position_ < source_.size() && is_identifier_continue(source_[position_])) {
    ++position_;
  }
  const auto end = position_;
  Token token{};
  token.leading_trivia = std::exchange(pending_leading_, {});
  token.lexeme = std::string(source_.substr(start, end - start));
  token.span = {start, end};
  token.kind = is_keyword(token.lexeme) ? TokenKind::Keyword : TokenKind::Identifier;
  return token;
}

Token Lexer::read_number() {
  const auto start = position_;
  while (position_ < source_.size() && is_digit(source_[position_])) {
    ++position_;
  }
  if (position_ < source_.size() && source_[position_] == '.') {
    ++position_;
    while (position_ < source_.size() && is_digit(source_[position_])) {
      ++position_;
    }
  }

  const auto end = position_;
  Token token{};
  token.leading_trivia = std::exchange(pending_leading_, {});
  token.lexeme = std::string(source_.substr(start, end - start));
  token.span = {start, end};
  token.kind = TokenKind::Number;
  return token;
}

Token Lexer::read_string() {
  const auto start = position_;
  ++position_;  // opening quote
  while (position_ < source_.size() && source_[position_] != '"') {
    if (source_[position_] == '\\' && position_ + 1 < source_.size()) {
      position_ += 2;
      continue;
    }
    ++position_;
  }

  if (position_ < source_.size()) {
    ++position_;  // closing quote
  }

  const auto end = position_;
  Token token{};
  token.leading_trivia = std::exchange(pending_leading_, {});
  token.lexeme = std::string(source_.substr(start, end - start));
  token.span = {start, end};
  token.kind = TokenKind::StringLiteral;
  return token;
}

Token Lexer::read_symbol() {
  const auto start = position_;
  std::string lexeme;
  lexeme.push_back(source_[position_]);
  ++position_;

  while (position_ < source_.size()) {
    std::string candidate = lexeme;
    candidate.push_back(source_[position_]);
    if (is_compound_symbol(candidate)) {
      lexeme = std::move(candidate);
      ++position_;
      continue;
    }
    break;
  }

  Token token{};
  token.leading_trivia = std::exchange(pending_leading_, {});
  token.lexeme = std::move(lexeme);
  token.span = {start, position_};
  token.kind = TokenKind::Symbol;
  return token;
}

Trivia Lexer::make_trivia(TriviaKind kind, std::size_t start, std::size_t end) const {
  Trivia trivia{};
  trivia.kind = kind;
  trivia.text = std::string(source_.substr(start, end - start));
  trivia.span = {start, end};
  return trivia;
}

void Lexer::skip_whitespace() {
  bool consumed = false;
  const auto start = position_;
  while (position_ < source_.size() &&
         std::isspace(static_cast<unsigned char>(source_[position_])) != 0) {
    ++position_;
    consumed = true;
  }
  if (consumed && config_.capture_whitespace) {
    pending_leading_.push_back(make_trivia(TriviaKind::Whitespace, start, position_));
  }
}

void Lexer::capture_trivia(TriviaKind kind, std::size_t start, std::size_t end, std::vector<Trivia>& target) {
  if ((kind == TriviaKind::Whitespace && !config_.capture_whitespace) ||
      (kind == TriviaKind::Comment && !config_.capture_comments)) {
    return;
  }
  target.push_back(make_trivia(kind, start, end));
}

TokenStream lex(std::string_view source, const LexerConfig& config) {
  Lexer lexer{source, config};
  return lexer.lex();
}

}  // namespace istudio::front
