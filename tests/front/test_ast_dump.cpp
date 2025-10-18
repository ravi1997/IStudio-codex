#include <stdexcept>
#include <string>

#include "front/ast_dump.h"
#include "front/lexer.h"
#include "front/parser.h"

using istudio::front::AstContext;
using istudio::front::AstDumpOptions;
using istudio::front::dump_ast_json;
using istudio::front::dump_ast_text;
using istudio::front::lex;
using istudio::front::parse_module;

namespace {

[[noreturn]] void fail(const std::string& message) {
  throw std::runtime_error(message);
}

void expect_equal(const std::string& actual, const std::string& expected, const std::string& message) {
  if (actual != expected) {
    fail(message + "\nExpected:\n" + expected + "\nActual:\n" + actual);
  }
}

std::string normalize_newlines(std::string value) {
  // Tests rely on deterministic newlines; ensure CRLF inputs from lexer don't interfere.
  std::string result{};
  result.reserve(value.size());
  for (char ch : value) {
    if (ch == '\r') {
      continue;
    }
    result.push_back(ch);
  }
  return result;
}

AstContext parse_source(const std::string& source, istudio::front::NodeId& root) {
  AstContext context{};
  istudio::front::LexerConfig config{};
  const auto tokens = lex(source, config);
  root = parse_module(tokens, context);
  return context;
}

void test_text_dump_simple_module() {
  istudio::front::NodeId root{};
  AstContext context = parse_source("let x = 1;", root);
  AstDumpOptions options{};
  options.include_ids = false;
  const std::string dump = dump_ast_text(context, root, options);
  const std::string expected = R"(Module span=[0, 10)
  LetStmt value="let" span=[0, 10)
    IdentifierExpr value="x" span=[4, 5)
    LiteralExpr value="1" span=[8, 9)
)";

  expect_equal(normalize_newlines(dump), expected, "AST text dump did not match expected output");
}

void test_json_dump_simple_module() {
  istudio::front::NodeId root{};
  AstContext context = parse_source("let x = 1;", root);
  const std::string dump = dump_ast_json(context, root, AstDumpOptions{});
  const std::string expected = R"({
  "id": 0,
  "kind": "Module",
  "span": {"start": 0, "end": 10},
  "value": "",
  "children": [
    {
      "id": 3,
      "kind": "LetStmt",
      "span": {"start": 0, "end": 10},
      "value": "let",
      "children": [
        {
          "id": 1,
          "kind": "IdentifierExpr",
          "span": {"start": 4, "end": 5},
          "value": "x",
          "children": []
        },
        {
          "id": 2,
          "kind": "LiteralExpr",
          "span": {"start": 8, "end": 9},
          "value": "1",
          "children": []
        }
      ]
    }
  ]
}
)";

  expect_equal(normalize_newlines(dump), expected, "AST JSON dump did not match expected output");
}

}  // namespace

void run_ast_dump_tests() {
  test_text_dump_simple_module();
  test_json_dump_simple_module();
}
