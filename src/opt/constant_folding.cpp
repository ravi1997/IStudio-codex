#include "opt/constant_folding.h"

#include <string>
#include <unordered_map>

namespace istudio::opt {
namespace {

void mark_constant(ir::IRValue& value, std::int64_t constant) {
  value.op = "const";
  value.operands.clear();
  value.is_constant = true;
  value.constant_value = constant;
}

bool try_parse_literal(const ir::IRValue& value, std::int64_t& out) {
  if (value.operands.empty()) {
    return false;
  }
  try {
    out = std::stoll(value.operands.front());
    return true;
  } catch (...) {
    return false;
  }
}

}  // namespace

void ConstantFoldingPass::run(ir::IRModule& module) {
  for (auto& function : module.functions()) {
    std::unordered_map<std::string, std::int64_t> constants{};
    for (auto& inst : function.instructions) {
      if (inst.is_constant) {
        constants[inst.result] = inst.constant_value;
        continue;
      }

      if (inst.op == "const") {
        std::int64_t literal{};
        if (try_parse_literal(inst, literal)) {
          mark_constant(inst, literal);
          constants[inst.result] = literal;
        }
        continue;
      }

      if (inst.operands.size() != 2) {
        continue;
      }

      const auto lhs_it = constants.find(inst.operands[0]);
      const auto rhs_it = constants.find(inst.operands[1]);
      if (lhs_it == constants.end() || rhs_it == constants.end()) {
        continue;
      }

      const auto lhs = lhs_it->second;
      const auto rhs = rhs_it->second;
      std::int64_t result{};

      if (inst.op == "add") {
        result = lhs + rhs;
      } else if (inst.op == "sub") {
        result = lhs - rhs;
      } else if (inst.op == "mul") {
        result = lhs * rhs;
      } else if (inst.op == "div" && rhs != 0) {
        result = lhs / rhs;
      } else {
        continue;
      }

      mark_constant(inst, result);
      constants[inst.result] = result;
    }
  }
}

}  // namespace istudio::opt
