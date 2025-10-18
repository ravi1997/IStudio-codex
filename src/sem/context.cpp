#include "sem/context.h"

#include <limits>
#include <string>
#include <string_view>
#include <utility>

namespace istudio::sem {

SymbolTable::SymbolTable() {
  push_scope();
}

void SymbolTable::push_scope() {
  scopes_.emplace_back();
}

void SymbolTable::pop_scope() {
  if (scopes_.size() > 1) {
    scopes_.pop_back();
  }
}

bool SymbolTable::insert(const std::string& name, front::NodeId id) {
  if (scopes_.empty()) {
    push_scope();
  }

  auto& scope = scopes_.back();
  return scope.emplace(name, id).second;
}

front::NodeId SymbolTable::lookup(const std::string& name) const {
  for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
    const auto found = it->find(name);
    if (found != it->end()) {
      return found->second;
    }
  }
  return std::numeric_limits<front::NodeId>::max();
}

std::pair<FunctionSignature*, bool> FunctionRegistry::declare(FunctionSignature signature) {
  auto [it, inserted] = by_name_.emplace(signature.name, std::move(signature));
  FunctionSignature* entry = &it->second;
  if (inserted) {
    by_node_.emplace(entry->node_id, entry);
  }
  return {entry, inserted};
}

FunctionSignature* FunctionRegistry::lookup(std::string_view name) {
  auto it = by_name_.find(std::string(name));
  if (it == by_name_.end()) {
    return nullptr;
  }
  return &it->second;
}

const FunctionSignature* FunctionRegistry::lookup(std::string_view name) const {
  auto it = by_name_.find(std::string(name));
  if (it == by_name_.end()) {
    return nullptr;
  }
  return &it->second;
}

FunctionSignature* FunctionRegistry::lookup(front::NodeId id) {
  auto it = by_node_.find(id);
  if (it == by_node_.end()) {
    return nullptr;
  }
  return it->second;
}

const FunctionSignature* FunctionRegistry::lookup(front::NodeId id) const {
  auto it = by_node_.find(id);
  if (it == by_node_.end()) {
    return nullptr;
  }
  return it->second;
}

SemanticContext::SemanticContext(SymbolTable table) : symbols_(std::move(table)) {}

}  // namespace istudio::sem
