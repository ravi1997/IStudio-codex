#pragma once

#include <memory>
#include <vector>

#include "ir/module.h"

namespace istudio::opt {

class Pass {
 public:
  virtual ~Pass() = default;
  virtual void run(ir::IRModule& module) = 0;
};

class PassManager {
 public:
  void add_pass(std::unique_ptr<Pass> pass);
  void run(ir::IRModule& module);

 private:
  std::vector<std::unique_ptr<Pass>> passes_{};
};

}  // namespace istudio::opt
