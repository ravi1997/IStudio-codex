#include "sem/context.h"

#include <limits>
#include <utility>

namespace istudio::sem {

bool SymbolTable::insert(const std::string& name, front::NodeId id) {
  return symbols_.emplace(name, id).second;
}

front::NodeId SymbolTable::lookup(const std::string& name) const {
  auto it = symbols_.find(name);
  if (it == symbols_.end()) {
    return std::numeric_limits<front::NodeId>::max();
  }
  return it->second;
}

SemanticContext::SemanticContext(SymbolTable table) : symbols_(std::move(table)) {}

}  // namespace istudio::sem
