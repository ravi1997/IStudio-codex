#include "front/parser.h"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

namespace istudio::front {
namespace {

support::Span merge_span(support::Span lhs, support::Span rhs) {
  return {.start = std::min(lhs.start, rhs.start), .end = std::max(lhs.end, rhs.end)};
}

}  // namespace

Parser::Parser(const TokenStream& tokens, AstContext& context)
    : tokens_(tokens), context_(context), index_(0) {}

NodeId Parser::parse_expression() {
  return parse_expression(1);
}

NodeId Parser::parse_expression(int min_precedence) {
  NodeId left = parse_prefix_expression();

  while (!at_end()) {
    const Token& op = current();
    const int precedence = precedence_for(op);

    if (precedence < min_precedence) {
      break;
    }

    if (op.kind != TokenKind::Symbol) {
      break;
    }

    advance();
    const bool is_assignment = is_assignment_operator(op);
    const int next_precedence = is_assignment ? precedence : precedence + 1;
    NodeId right = parse_expression(next_precedence);

    const auto& left_node = context_.node(left);
    const auto& right_node = context_.node(right);
    support::Span span = merge_span(left_node.span, right_node.span);

    AstKind kind = is_assignment ? AstKind::AssignmentExpr : AstKind::BinaryExpr;
    auto& expr = context_.create_node(kind, span, op.lexeme);
    expr.children.push_back(left);
    expr.children.push_back(right);
    left = expr.id;
  }

  return left;
}

NodeId Parser::parse_prefix_expression() {
  if (at_end()) {
    throw std::runtime_error("unexpected end of input");
  }

  const Token& token = current();
  if (is_unary_prefix(token)) {
    const Token& op = advance();
    NodeId operand = parse_expression(precedence_for(op));
    const auto& operand_node = context_.node(operand);
    support::Span span = merge_span(op.span, operand_node.span);
    auto& expr = context_.create_node(AstKind::UnaryExpr, span, op.lexeme);
    expr.children.push_back(operand);
    return expr.id;
  }

  NodeId primary = parse_primary_expression();
  const auto& primary_node = context_.node(primary);
  return parse_call_expression(primary, primary_node.span);
}

NodeId Parser::parse_primary_expression() {
  if (at_end()) {
    throw std::runtime_error("unexpected end of input");
  }

  const Token& token = advance();
  switch (token.kind) {
    case TokenKind::Identifier: {
      auto& node = context_.create_node(AstKind::IdentifierExpr, token.span, token.lexeme);
      return node.id;
    }
    case TokenKind::Number:
    case TokenKind::StringLiteral: {
      auto& node = context_.create_node(AstKind::LiteralExpr, token.span, token.lexeme);
      return node.id;
    }
    case TokenKind::Keyword: {
      auto& node = context_.create_node(AstKind::LiteralExpr, token.span, token.lexeme);
      return node.id;
    }
    case TokenKind::Symbol: {
      if (token.lexeme == "(") {
        NodeId expr = parse_expression();
        const Token& closing = consume_symbol(")", "expected ')' after expression");
        support::Span span = merge_span(token.span, closing.span);
        auto& group = context_.create_node(AstKind::GroupExpr, span);
        group.children.push_back(expr);
        return group.id;
      }
      break;
    }
    default:
      break;
  }

  throw std::runtime_error("unexpected token in primary expression");
}

NodeId Parser::parse_call_expression(NodeId callee, support::Span callee_span) {
  NodeId current_callee = callee;
  support::Span current_span = callee_span;

  while (!at_end()) {
    if (match_symbol("(")) {
      std::vector<NodeId> args{};
      if (!check_symbol(")")) {
        do {
          NodeId expr = parse_expression();
          args.push_back(expr);
        } while (match_symbol(","));
      }

      const Token& close = consume_symbol(")", "expected ')' after arguments");
      support::Span span = merge_span(current_span, close.span);
      auto& call = context_.create_node(AstKind::CallExpr, span);
      call.children.push_back(current_callee);
      for (auto arg : args) {
        call.children.push_back(arg);
      }

      current_callee = call.id;
      current_span = span;
      continue;
    }
    break;
  }

  return current_callee;
}

bool Parser::match_symbol(std::string_view symbol) {
  if (check_symbol(symbol)) {
    advance();
    return true;
  }
  return false;
}

bool Parser::check_symbol(std::string_view symbol) const {
  if (at_end()) {
    return false;
  }
  const Token& token = current();
  return token.kind == TokenKind::Symbol && token.lexeme == symbol;
}

const Token& Parser::consume_symbol(std::string_view symbol, std::string_view message) {
  if (!check_symbol(symbol)) {
    throw std::runtime_error(std::string(message));
  }
  return advance();
}

const Token& Parser::advance() {
  if (!at_end()) {
    ++index_;
  }
  return previous();
}

const Token& Parser::current() const {
  return tokens_.tokens[index_];
}

const Token& Parser::peek(std::size_t offset) const {
  const std::size_t position = std::min(tokens_.tokens.size() - 1, index_ + offset);
  return tokens_.tokens[position];
}

const Token& Parser::previous() const {
  const std::size_t position = index_ == 0 ? 0 : index_ - 1;
  return tokens_.tokens[position];
}

bool Parser::at_end() const {
  return index_ >= tokens_.tokens.size() || current().kind == TokenKind::EndOfFile;
}

int Parser::precedence_for(const Token& token) const {
  if (token.kind != TokenKind::Symbol) {
    return -1;
  }

  const std::string& symbol = token.lexeme;
  if (symbol == "=") {
    return 1;
  }
  if (symbol == "||") {
    return 2;
  }
  if (symbol == "&&") {
    return 3;
  }
  if (symbol == "==" || symbol == "!=") {
    return 4;
  }
  if (symbol == "<" || symbol == ">" || symbol == "<=" || symbol == ">=") {
    return 5;
  }
  if (symbol == "+" || symbol == "-") {
    return 6;
  }
  if (symbol == "*" || symbol == "/" || symbol == "%") {
    return 7;
  }
  return -1;
}

bool Parser::is_assignment_operator(const Token& token) const {
  return token.kind == TokenKind::Symbol && (token.lexeme == "=" || token.lexeme == "+=" || token.lexeme == "-=" ||
                                             token.lexeme == "*=" || token.lexeme == "/=");
}

bool Parser::is_unary_prefix(const Token& token) const {
  if (token.kind == TokenKind::Symbol) {
    return token.lexeme == "!" || token.lexeme == "-" || token.lexeme == "+";
  }
  if (token.kind == TokenKind::Keyword) {
    return token.lexeme == "await";
  }
  return false;
}

NodeId parse_expression(const TokenStream& tokens, AstContext& context) {
  Parser parser(tokens, context);
  return parser.parse_expression();
}

}  // namespace istudio::front
