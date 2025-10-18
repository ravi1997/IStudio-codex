#pragma once

#include <limits>

#include "front/ast.h"

namespace istudio::sem {

enum class TypeKind {
  Unknown,
  Void,
  Integer,
  Float,
  Bool,
  String,
  Function,
};

struct Type {
  TypeKind kind{TypeKind::Unknown};
  front::NodeId reference{std::numeric_limits<front::NodeId>::max()};
};

}  // namespace istudio::sem

