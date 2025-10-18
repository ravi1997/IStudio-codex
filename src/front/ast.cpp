#include "front/ast.h"

#include <stdexcept>
#include <utility>

namespace istudio::front {

AstNode& AstContext::create_node(AstKind kind, support::Span span, std::string value) {
  const NodeId id = nodes_.size();
  nodes_.push_back(AstNode{.id = id, .kind = kind, .span = span, .value = std::move(value), .children = {}});
  return nodes_.back();
}

const AstNode& AstContext::node(NodeId id) const {
  if (id >= nodes_.size()) {
    throw std::out_of_range("invalid AstNode id");
  }
  return nodes_[id];
}

AstNode& AstContext::node(NodeId id) {
  if (id >= nodes_.size()) {
    throw std::out_of_range("invalid AstNode id");
  }
  return nodes_[id];
}

std::string_view to_string(AstKind kind) noexcept {
  switch (kind) {
    case AstKind::Unknown:
      return "Unknown";
    case AstKind::Module:
      return "Module";
    case AstKind::Function:
      return "Function";
    case AstKind::Declaration:
      return "Declaration";
    case AstKind::Expression:
      return "Expression";
    case AstKind::AssignmentExpr:
      return "AssignmentExpr";
    case AstKind::BinaryExpr:
      return "BinaryExpr";
    case AstKind::UnaryExpr:
      return "UnaryExpr";
    case AstKind::LiteralExpr:
      return "LiteralExpr";
    case AstKind::IdentifierExpr:
      return "IdentifierExpr";
    case AstKind::CallExpr:
      return "CallExpr";
    case AstKind::ArgumentList:
      return "ArgumentList";
    case AstKind::GroupExpr:
      return "GroupExpr";
    case AstKind::BlockStmt:
      return "BlockStmt";
    case AstKind::LetStmt:
      return "LetStmt";
    case AstKind::ReturnStmt:
      return "ReturnStmt";
    case AstKind::ExpressionStmt:
      return "ExpressionStmt";
  }

  return "Unknown";
}

}  // namespace istudio::front
