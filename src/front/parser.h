#pragma once

#include <cstddef>
#include <string_view>

#include "front/ast.h"
#include "front/token.h"

namespace istudio::front {

class Parser {
 public:
  Parser(const TokenStream& tokens, AstContext& context);

  NodeId parse_expression();

 private:
  NodeId parse_expression(int min_precedence);
  NodeId parse_prefix_expression();
  NodeId parse_primary_expression();
  NodeId parse_call_expression(NodeId callee, support::Span callee_span);

  bool match_symbol(std::string_view symbol);
  bool check_symbol(std::string_view symbol) const;
  const Token& consume_symbol(std::string_view symbol, std::string_view message);

  const Token& advance();
  const Token& current() const;
  const Token& peek(std::size_t offset) const;
  const Token& previous() const;
  bool at_end() const;

  int precedence_for(const Token& token) const;
  bool is_assignment_operator(const Token& token) const;
  bool is_unary_prefix(const Token& token) const;

  const TokenStream& tokens_;
  AstContext& context_;
  std::size_t index_{0};
};

NodeId parse_expression(const TokenStream& tokens, AstContext& context);

}  // namespace istudio::front
