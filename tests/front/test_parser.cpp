#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

#include "front/lexer.h"
#include "front/parser.h"

using istudio::front::AstContext;
using istudio::front::AstKind;
using istudio::front::LexerConfig;
using istudio::front::NodeId;
using istudio::front::TokenKind;
using istudio::front::lex;
using istudio::front::parse_expression;

namespace {

[[noreturn]] void fail(const std::string& message) {
  throw std::runtime_error(message);
}

void expect(bool condition, const std::string& message) {
  if (!condition) {
    fail(message);
  }
}

NodeId parse_expr(const std::string& source, AstContext& context) {
  LexerConfig config{};
  const auto tokens = lex(source, config);
  return parse_expression(tokens, context);
}

void test_assignment_and_precedence() {
  AstContext context{};
  const NodeId root = parse_expr("a = 1 + 2 * 3", context);
  const auto& root_node = context.node(root);
  expect(root_node.kind == AstKind::AssignmentExpr, "root should be assignment expr");
  expect(root_node.value == "=", "assignment operator should be '='");
  expect(root_node.children.size() == 2, "assignment should have two children");

  const auto& left = context.node(root_node.children[0]);
  expect(left.kind == AstKind::IdentifierExpr, "left side should be identifier");
  expect(left.value == "a", "identifier should be 'a'");

  const auto& right = context.node(root_node.children[1]);
  expect(right.kind == AstKind::BinaryExpr, "right side should be binary expression");
  expect(right.value == "+", "binary operator should be '+'");
  expect(right.children.size() == 2, "binary node must have two children");

  const auto& rhs_left = context.node(right.children[0]);
  expect(rhs_left.kind == AstKind::LiteralExpr, "lhs of '+' should be literal");
  expect(rhs_left.value == "1", "literal should be '1'");

  const auto& rhs_right = context.node(right.children[1]);
  expect(rhs_right.kind == AstKind::BinaryExpr, "rhs of '+' should be binary '*'");
  expect(rhs_right.value == "*", "inner operator should be '*'");
}

void test_grouping_and_multiplication() {
  AstContext context{};
  const NodeId root = parse_expr("(1 + 2) * 3", context);
  const auto& node = context.node(root);
  expect(node.kind == AstKind::BinaryExpr, "root should be binary expr");
  expect(node.value == "*", "root operator should be '*'");

  const auto& left = context.node(node.children[0]);
  expect(left.kind == AstKind::GroupExpr, "left operand should be group expr");
  expect(left.children.size() == 1, "group should contain inner expr");
  const auto& inner = context.node(left.children[0]);
  expect(inner.kind == AstKind::BinaryExpr, "group inner should be binary '+'");
  expect(inner.value == "+", "inner operator should be '+'");
}

void test_call_expression() {
  AstContext context{};
  const NodeId root = parse_expr("add(1, 2 * 3)", context);
  const auto& node = context.node(root);
  expect(node.kind == AstKind::CallExpr, "root should be call expr");
  expect(node.children.size() == 3, "call should contain callee and two args");

  const auto& callee = context.node(node.children[0]);
  expect(callee.kind == AstKind::IdentifierExpr, "callee should be identifier");
  expect(callee.value == "add", "callee name should be add");

  const auto& arg0 = context.node(node.children[1]);
  expect(arg0.kind == AstKind::LiteralExpr, "first arg should be literal");
  expect(arg0.value == "1", "first arg should be '1'");

  const auto& arg1 = context.node(node.children[2]);
  expect(arg1.kind == AstKind::BinaryExpr, "second arg should be binary '*'");
  expect(arg1.value == "*", "second arg operator should be '*'");
}

void test_unary_expression() {
  AstContext context{};
  const NodeId root = parse_expr("-value", context);
  const auto& node = context.node(root);
  expect(node.kind == AstKind::UnaryExpr, "root should be unary expr");
  expect(node.value == "-", "unary operator should be '-'");
  expect(node.children.size() == 1, "unary expr must have operand");
  const auto& operand = context.node(node.children[0]);
  expect(operand.kind == AstKind::IdentifierExpr, "operand should be identifier");
}

}  // namespace

void run_parser_tests() {
  test_assignment_and_precedence();
  test_grouping_and_multiplication();
  test_call_expression();
  test_unary_expression();
  std::cout << "All parser tests passed\n";
}

