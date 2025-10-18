#include <iostream>
#include <stdexcept>
#include <string>

#include "ir/module.h"
#include "ir/printer.h"
#include "opt/constant_folding.h"

using istudio::ir::IRModule;
using istudio::ir::IRValue;
using istudio::ir::print_module;
using istudio::opt::ConstantFoldingPass;

namespace {

[[noreturn]] void fail(const std::string& message) {
  throw std::runtime_error(message);
}

void expect(bool condition, const std::string& message) {
  if (!condition) {
    fail(message);
  }
}

IRModule build_sample_module() {
  IRModule module;
  auto& fn = module.add_function("main");
  fn.add_instruction(IRValue{.result = "c1", .op = "const", .operands = {"2"}});
  fn.add_instruction(IRValue{.result = "c2", .op = "const", .operands = {"3"}});
  fn.add_instruction(IRValue{.result = "sum", .op = "add", .operands = {"c1", "c2"}});
  return module;
}

void test_constant_folding_pass() {
  auto module = build_sample_module();
  ConstantFoldingPass pass{};
  pass.run(module);

  const auto& fn = module.functions().front();
  expect(fn.instructions.size() == 3, "function should retain three instructions");
  const auto& folded = fn.instructions.back();
  expect(folded.is_constant, "sum should be folded to constant");
  expect(folded.constant_value == 5, "folded constant should equal 5");
}

void test_ir_printer_outputs_text() {
  auto module = build_sample_module();
  ConstantFoldingPass pass{};
  pass.run(module);
  const auto text = print_module(module);
  expect(text.find("function main") != std::string::npos, "printer should include function header");
  expect(text.find("sum = const 5") != std::string::npos, "printer should show folded constant");
}

}  // namespace

void run_ir_tests() {
  test_constant_folding_pass();
  test_ir_printer_outputs_text();
  std::cout << "All IR tests passed\n";
}

