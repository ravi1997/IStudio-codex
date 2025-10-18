#pragma once

#include <string>
#include <utility>
#include <vector>

namespace istudio::ir {

enum class IRTypeKind {
  Void,
  I32,
  I64,
  F32,
  F64,
  Bool,
  String,
  Struct,
  Generic,
};

struct IRType {
  IRTypeKind kind{IRTypeKind::Void};
  std::string name{};
  std::vector<IRType> type_arguments{};

  static IRType Void() { return IRType{IRTypeKind::Void, {}, {}}; }
  static IRType I32() { return IRType{IRTypeKind::I32, {}, {}}; }
  static IRType I64() { return IRType{IRTypeKind::I64, {}, {}}; }
  static IRType F32() { return IRType{IRTypeKind::F32, {}, {}}; }
  static IRType F64() { return IRType{IRTypeKind::F64, {}, {}}; }
  static IRType Bool() { return IRType{IRTypeKind::Bool, {}, {}}; }
  static IRType String() { return IRType{IRTypeKind::String, {}, {}}; }
  static IRType Struct(std::string name, std::vector<IRType> type_arguments = {}) {
    return IRType{IRTypeKind::Struct, std::move(name), std::move(type_arguments)};
  }
  static IRType Generic(std::string name) {
    return IRType{IRTypeKind::Generic, std::move(name), {}};
  }

  [[nodiscard]] bool is_struct() const noexcept { return kind == IRTypeKind::Struct; }
  [[nodiscard]] bool is_generic() const noexcept { return kind == IRTypeKind::Generic; }
  [[nodiscard]] bool is_builtin() const noexcept {
    return kind == IRTypeKind::Void || kind == IRTypeKind::I32 || kind == IRTypeKind::I64 ||
           kind == IRTypeKind::F32 || kind == IRTypeKind::F64 || kind == IRTypeKind::Bool ||
           kind == IRTypeKind::String;
  }

  friend bool operator==(const IRType& lhs, const IRType& rhs) {
    return lhs.kind == rhs.kind && lhs.name == rhs.name &&
           lhs.type_arguments == rhs.type_arguments;
  }

  friend bool operator!=(const IRType& lhs, const IRType& rhs) { return !(lhs == rhs); }
};

}  // namespace istudio::ir

