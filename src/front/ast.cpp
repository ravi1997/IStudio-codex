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

}  // namespace istudio::front
