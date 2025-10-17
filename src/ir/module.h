#pragma once

#include <string>
#include <vector>

namespace istudio::ir {

struct IRValue {
  std::string name{};
};

struct IRFunction {
  std::string name{};
  std::vector<IRValue> body{};
};

class IRModule {
 public:
  IRModule() = default;

  IRFunction& add_function(std::string name);
  [[nodiscard]] const std::vector<IRFunction>& functions() const noexcept { return functions_; }

 private:
  std::vector<IRFunction> functions_{};
};

}  // namespace istudio::ir
