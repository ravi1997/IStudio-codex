#include "ir/module.h"

#include <utility>

namespace istudio::ir {

IRFunction& IRModule::add_function(std::string name) {
  functions_.push_back(IRFunction{.name = std::move(name), .body = {}});
  return functions_.back();
}

}  // namespace istudio::ir
