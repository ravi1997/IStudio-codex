#pragma once

#include <memory>
#include <string>

#include "ir/module.h"

namespace istudio::backends {

struct TargetProfile {
  std::string name{};
  std::string version{};
};

class Backend {
 public:
  virtual ~Backend() = default;
  [[nodiscard]] virtual std::string name() const = 0;
  virtual void emit(const ir::IRModule& module, const TargetProfile& profile) = 0;
};

using BackendPtr = std::unique_ptr<Backend>;

}  // namespace istudio::backends
