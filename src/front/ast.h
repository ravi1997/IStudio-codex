#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "support/span.h"

namespace istudio::front {

using NodeId = std::size_t;

enum class AstKind {
  Unknown,
  Module,
  Function,
  Declaration,
  Expression,
  AssignmentExpr,
  BinaryExpr,
  UnaryExpr,
  LiteralExpr,
  IdentifierExpr,
  CallExpr,
  ArgumentList,
  GroupExpr,
  BlockStmt,
  LetStmt,
  ReturnStmt,
  ExpressionStmt,
};

struct AstNode {
  NodeId id{0};
  AstKind kind{AstKind::Unknown};
  support::Span span{};
  std::string value{};
  std::vector<NodeId> children{};
};

class AstContext {
 public:
  AstContext() = default;

  [[nodiscard]] AstNode& create_node(AstKind kind, support::Span span, std::string value = {});
  [[nodiscard]] const AstNode& node(NodeId id) const;
  [[nodiscard]] AstNode& node(NodeId id);
  [[nodiscard]] std::size_t size() const noexcept { return nodes_.size(); }

 private:
  std::vector<AstNode> nodes_{};
};

[[nodiscard]] std::string_view to_string(AstKind kind) noexcept;

}  // namespace istudio::front
