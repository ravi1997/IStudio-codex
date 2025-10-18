#include "ir/printer.h"

#include <sstream>
#include <string_view>

namespace istudio::ir {
namespace {

std::string join_operands(const std::vector<std::string>& operands) {
  std::ostringstream oss;
  for (std::size_t i = 0; i < operands.size(); ++i) {
    if (i != 0) {
      oss << ", ";
    }
    oss << operands[i];
  }
  return oss.str();
}

}  // namespace

std::string print_module(const IRModule& module) {
  std::ostringstream oss;

  for (const auto& function : module.functions()) {
    oss << "function " << function.name << " {\n";
    for (const auto& inst : function.instructions) {
      oss << "  " << inst.result << " = ";
      if (inst.is_constant) {
        oss << "const " << inst.constant_value;
      } else {
        oss << inst.op;
        if (!inst.operands.empty()) {
          oss << " " << join_operands(inst.operands);
        }
      }
      oss << ";\n";
    }
    oss << "}\n";
  }

  return oss.str();
}

}  // namespace istudio::ir
