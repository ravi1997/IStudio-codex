#pragma once

#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "front/ast.h"
#include "sem/types.h"

namespace istudio::sem {

class SymbolTable {
 public:
  SymbolTable();

  void push_scope();
  void pop_scope();
  [[nodiscard]] std::size_t depth() const noexcept { return scopes_.size(); }

  bool insert(const std::string& name, front::NodeId id);
  [[nodiscard]] front::NodeId lookup(const std::string& name) const;

 private:
  std::vector<std::unordered_map<std::string, front::NodeId>> scopes_{};
};

struct FunctionParameter {
  std::string name{};
  front::NodeId node_id{std::numeric_limits<front::NodeId>::max()};
  Type type{};
};

struct FunctionSignature {
  std::string name{};
  front::NodeId node_id{std::numeric_limits<front::NodeId>::max()};
  std::vector<FunctionParameter> parameters{};
  Type return_type{};
};

class FunctionRegistry {
 public:
  FunctionRegistry() = default;

  std::pair<FunctionSignature*, bool> declare(FunctionSignature signature);
  [[nodiscard]] FunctionSignature* lookup(std::string_view name);
  [[nodiscard]] const FunctionSignature* lookup(std::string_view name) const;
  [[nodiscard]] FunctionSignature* lookup(front::NodeId id);
  [[nodiscard]] const FunctionSignature* lookup(front::NodeId id) const;
  [[nodiscard]] const std::unordered_map<std::string, FunctionSignature>& entries() const noexcept {
    return by_name_;
  }

 private:
  std::unordered_map<std::string, FunctionSignature> by_name_{};
  std::unordered_map<front::NodeId, FunctionSignature*> by_node_{};
};

class SemanticContext {
 public:
  explicit SemanticContext(SymbolTable table = {});

  [[nodiscard]] SymbolTable& symbols() noexcept { return symbols_; }
  [[nodiscard]] const SymbolTable& symbols() const noexcept { return symbols_; }

  [[nodiscard]] FunctionRegistry& functions() noexcept { return functions_; }
  [[nodiscard]] const FunctionRegistry& functions() const noexcept { return functions_; }

 private:
  SymbolTable symbols_{};
  FunctionRegistry functions_{};
};

}  // namespace istudio::sem
