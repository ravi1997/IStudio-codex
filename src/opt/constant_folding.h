#pragma once

#include "opt/pass_manager.h"

namespace istudio::opt {

class ConstantFoldingPass : public Pass {
 public:
  void run(ir::IRModule& module) override;
};

}  // namespace istudio::opt
