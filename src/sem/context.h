#pragma once

#include <string>
#include <unordered_map>

#include "front/ast.h"

namespace istudio::sem {

class SymbolTable {
 public:
  bool insert(const std::string& name, front::NodeId id);
  [[nodiscard]] front::NodeId lookup(const std::string& name) const;

 private:
  std::unordered_map<std::string, front::NodeId> symbols_{};
};

class SemanticContext {
 public:
  explicit SemanticContext(SymbolTable table = {});

  [[nodiscard]] SymbolTable& symbols() noexcept { return symbols_; }
  [[nodiscard]] const SymbolTable& symbols() const noexcept { return symbols_; }

 private:
  SymbolTable symbols_{};
};

}  // namespace istudio::sem
