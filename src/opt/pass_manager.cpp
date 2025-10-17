#include "opt/pass_manager.h"

#include <utility>

namespace istudio::opt {

void PassManager::add_pass(std::unique_ptr<Pass> pass) {
  passes_.push_back(std::move(pass));
}

void PassManager::run(ir::IRModule& module) {
  for (auto& pass : passes_) {
    pass->run(module);
  }
}

}  // namespace istudio::opt
