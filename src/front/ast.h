#pragma once

#include <cstddef>
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
};

struct AstNode {
  NodeId id{0};
  AstKind kind{AstKind::Unknown};
  support::Span span{};
  std::vector<NodeId> children{};
};

class AstContext {
 public:
  AstContext() = default;

  [[nodiscard]] AstNode& create_node(AstKind kind, support::Span span);
  [[nodiscard]] const AstNode& node(NodeId id) const;
  [[nodiscard]] std::size_t size() const noexcept { return nodes_.size(); }

 private:
  std::vector<AstNode> nodes_{};
};

}  // namespace istudio::front
