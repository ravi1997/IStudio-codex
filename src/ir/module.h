#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "ir/type.h"

namespace istudio::ir {

struct IRValue {
  std::string result{};
  std::string op{};
  std::vector<std::string> operands{};
  bool is_constant{false};
  std::int64_t constant_value{0};
};

struct IRParameter {
  std::string name{};
  IRType type{IRType::Void()};
};

struct IRField {
  std::string name{};
  IRType type{IRType::Void()};
};

struct IRStruct {
  std::string name{};
  std::vector<std::string> template_params{};
  std::vector<IRField> fields{};
  bool is_public{true};
};

struct IRFunction {
  std::string name{};
  IRType return_type{IRType::Void()};
  std::vector<std::string> template_params{};
  std::vector<IRParameter> parameters{};
  std::vector<IRValue> instructions{};

  IRValue& add_instruction(IRValue value);
};

class IRModule {
 public:
  explicit IRModule(std::string name = "module") : name_(std::move(name)) {}

  void set_name(std::string name) { name_ = std::move(name); }
  [[nodiscard]] const std::string& name() const noexcept { return name_; }

  IRStruct& add_struct(IRStruct value);
  IRStruct& add_struct(std::string name, std::vector<IRField> fields = {},
                       std::vector<std::string> template_params = {}, bool is_public = true);
  [[nodiscard]] const std::vector<IRStruct>& structs() const noexcept { return structs_; }
  [[nodiscard]] std::vector<IRStruct>& structs() noexcept { return structs_; }

  IRFunction& add_function(std::string name, IRType return_type = IRType::Void(),
                           std::vector<IRParameter> parameters = {},
                           std::vector<std::string> template_params = {});
  IRFunction& add_function(IRFunction function);
  [[nodiscard]] const std::vector<IRFunction>& functions() const noexcept { return functions_; }
  [[nodiscard]] std::vector<IRFunction>& functions() noexcept { return functions_; }

 private:
  std::string name_;
  std::vector<IRStruct> structs_{};
  std::vector<IRFunction> functions_{};
};

}  // namespace istudio::ir
