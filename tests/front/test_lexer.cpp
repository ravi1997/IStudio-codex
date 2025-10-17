#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

#include "front/lexer.h"

using istudio::front::lex;
using istudio::front::LexerConfig;
using istudio::front::TokenKind;
using istudio::front::TriviaKind;

namespace {

void expect(bool condition, const std::string& message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

void test_tokenizes_keywords_identifiers_and_symbols() {
  const std::string source = "module demo\nfn main() {\n  return 42\n}\n";

  LexerConfig config{};
  config.capture_whitespace = false;
  config.capture_comments = true;

  const auto stream = lex(source, config);
  expect(!stream.tokens.empty(), "Token stream should not be empty");

  std::vector<TokenKind> kinds{};
  std::vector<std::string> lexemes{};
  kinds.reserve(stream.tokens.size());
  lexemes.reserve(stream.tokens.size());

  for (const auto& token : stream.tokens) {
    kinds.push_back(token.kind);
    lexemes.push_back(token.lexeme);
  }

  expect(kinds.front() == TokenKind::Keyword, "First token must be keyword");
  expect(lexemes.front() == "module", "First lexeme must be 'module'");
  expect(kinds.back() == TokenKind::EndOfFile, "Last token must be EOF");
  expect(lexemes[1] == "demo", "Second lexeme must be module name");
  expect(std::find(kinds.begin(), kinds.end(), TokenKind::Identifier) != kinds.end(),
         "Should encounter identifiers");
  expect(std::find(kinds.begin(), kinds.end(), TokenKind::Symbol) != kinds.end(),
         "Should encounter symbols");
}

void test_captures_trivia_when_enabled() {
  const std::string source = "  let x = 1\n// trailing comment\n";

  LexerConfig config{};
  config.capture_whitespace = true;
  config.capture_comments = true;

  const auto stream = lex(source, config);
  expect(stream.tokens.size() >= 2, "Token stream should contain at least two tokens");

  const auto& first = stream.tokens[0];
  expect(first.kind == TokenKind::Keyword, "First token should be keyword 'let'");
  expect(first.lexeme == "let", "Lexeme should be 'let'");
  expect(first.leading_trivia.size() == 1, "Leading trivia should contain whitespace");
  expect(first.leading_trivia[0].kind == TriviaKind::Whitespace, "Trivia should be whitespace");
  expect(first.leading_trivia[0].text == "  ", "Whitespace trivia should capture indentation");

  const auto& eof = stream.tokens.back();
  expect(eof.kind == TokenKind::EndOfFile, "Last token should be EOF");
  const bool has_comment = std::any_of(
      eof.leading_trivia.begin(),
      eof.leading_trivia.end(),
      [](const auto& trivia) { return trivia.kind == TriviaKind::Comment; });
  expect(has_comment, "EOF leading trivia should include trailing comment");
}

}  // namespace

int main() {
  try {
    test_tokenizes_keywords_identifiers_and_symbols();
    test_captures_trivia_when_enabled();
  } catch (const std::exception& ex) {
    std::cerr << "[lexer] " << ex.what() << '\n';
    return EXIT_FAILURE;
  }

  std::cout << "All lexer tests passed\n";
  return EXIT_SUCCESS;
}
