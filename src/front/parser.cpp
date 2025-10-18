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

support::Span span_covering(const TokenStream& tokens) {
  if (tokens.tokens.empty()) {
    return {};
  }
  support::Span span = tokens.tokens.front().span;
  span.end = tokens.tokens.back().span.end;
  return span;
}

}  // namespace

Parser::Parser(const TokenStream& tokens, AstContext& context)
    : tokens_(tokens), context_(context), index_(0) {}

NodeId Parser::parse_module() {
  auto& module_ref = context_.create_node(AstKind::Module, span_covering(tokens_));
  const NodeId module_id = module_ref.id;

  while (!at_end() && current().kind != TokenKind::EndOfFile) {
    NodeId stmt = parse_statement();
    context_.node(module_id).children.push_back(stmt);
  }

  return module_id;
}

NodeId Parser::parse_expression() {
  return parse_expression(1);
}

NodeId Parser::parse_statement() {
  if (check_keyword("let")) {
    return parse_let_statement();
  }
  if (check_keyword("return")) {
    return parse_return_statement();
  }
  if (check_symbol("{")) {
    return parse_block_statement();
  }

  NodeId expr = parse_expression();
  const Token& semi = consume_symbol(";", "expected ';' after expression");
  const auto& expr_node = context_.node(expr);
  auto& stmt = context_.create_node(AstKind::ExpressionStmt, merge_span(expr_node.span, semi.span));
  stmt.children.push_back(expr);
  return stmt.id;
}

NodeId Parser::parse_block_statement() {
  const Token& open = consume_symbol("{", "expected '{'");
  auto& block_ref = context_.create_node(AstKind::BlockStmt, open.span);
  const NodeId block_id = block_ref.id;

  while (!at_end() && !check_symbol("}")) {
    NodeId stmt = parse_statement();
    context_.node(block_id).children.push_back(stmt);
  }

  const Token& close = consume_symbol("}", "expected '}' to close block");
  auto& block = context_.node(block_id);
  block.span = merge_span(open.span, close.span);
  return block_id;
}

NodeId Parser::parse_let_statement() {
  const Token& let_token = consume_keyword("let", "expected 'let'");
  bool is_mutable = match_keyword("mut");

  const Token& ident = consume_identifier("expected identifier after 'let'");
  auto& name_node = context_.create_node(AstKind::IdentifierExpr, ident.span, ident.lexeme);

  consume_symbol("=", "expected '=' in let binding");
  NodeId initializer = parse_expression();
  const Token& semi = consume_symbol(";", "expected ';' after let binding");

  auto& let_node =
      context_.create_node(AstKind::LetStmt, merge_span(let_token.span, semi.span), is_mutable ? "mut" : "let");
  let_node.children.push_back(name_node.id);
  let_node.children.push_back(initializer);
  return let_node.id;
}

NodeId Parser::parse_return_statement() {
  const Token& return_token = consume_keyword("return", "expected 'return'");
  bool has_value = !check_symbol(";");
  NodeId value{};
  if (has_value) {
    value = parse_expression();
  }
  const Token& semi = consume_symbol(";", "expected ';' after return");

  auto& return_node = context_.create_node(AstKind::ReturnStmt, merge_span(return_token.span, semi.span));
  if (has_value) {
    return_node.children.push_back(value);
  }
  return return_node.id;
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

bool Parser::match_keyword(std::string_view keyword) {
  if (check_keyword(keyword)) {
    advance();
    return true;
  }
  return false;
}

bool Parser::check_keyword(std::string_view keyword) const {
  if (at_end()) {
    return false;
  }
  const Token& token = current();
  return token.kind == TokenKind::Keyword && token.lexeme == keyword;
}

const Token& Parser::consume_keyword(std::string_view keyword, std::string_view message) {
  if (!check_keyword(keyword)) {
    throw std::runtime_error(std::string(message));
  }
  return advance();
}

const Token& Parser::consume_identifier(std::string_view message) {
  if (at_end() || current().kind != TokenKind::Identifier) {
    throw std::runtime_error(std::string(message));
  }
  return advance();
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

NodeId parse_module(const TokenStream& tokens, AstContext& context) {
  Parser parser(tokens, context);
  return parser.parse_module();
}

NodeId parse_expression(const TokenStream& tokens, AstContext& context) {
  Parser parser(tokens, context);
  return parser.parse_expression();
}

}  // namespace istudio::front
