#include "ir/module.h"

#include <utility>

namespace istudio::ir {

IRValue& IRFunction::add_instruction(IRValue value) {
  instructions.push_back(std::move(value));
  return instructions.back();
}

IRStruct& IRModule::add_struct(IRStruct value) {
  structs_.push_back(std::move(value));
  return structs_.back();
}

IRStruct& IRModule::add_struct(std::string name, std::vector<IRField> fields,
                               std::vector<std::string> template_params, bool is_public) {
  IRStruct value{};
  value.name = std::move(name);
  value.template_params = std::move(template_params);
  value.fields = std::move(fields);
  value.is_public = is_public;
  return add_struct(std::move(value));
}

IRFunction& IRModule::add_function(std::string name, IRType return_type,
                                   std::vector<IRParameter> parameters,
                                   std::vector<std::string> template_params) {
  IRFunction fn{};
  fn.name = std::move(name);
  fn.return_type = std::move(return_type);
  fn.parameters = std::move(parameters);
  fn.template_params = std::move(template_params);
  return add_function(std::move(fn));
}

IRFunction& IRModule::add_function(IRFunction function) {
  functions_.push_back(std::move(function));
  return functions_.back();
}

}  // namespace istudio::ir
