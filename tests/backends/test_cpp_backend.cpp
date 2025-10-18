#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "backends/cpp/cpp_backend.h"
#include "ir/module.h"

using istudio::backends::GeneratedFile;
using istudio::backends::TargetProfile;
using istudio::backends::cpp::CppBackend;
using istudio::ir::IRField;
using istudio::ir::IRModule;
using istudio::ir::IRParameter;
using istudio::ir::IRType;
using istudio::ir::IRValue;

namespace {

[[noreturn]] void fail(const std::string& message) {
  throw std::runtime_error(message);
}

void expect(bool condition, const std::string& message) {
  if (!condition) {
    fail(message);
  }
}

const GeneratedFile* find_file(const std::vector<GeneratedFile>& files, std::string_view suffix) {
  for (const auto& file : files) {
    if (file.path == suffix) {
      return &file;
    }
  }
  fail("expected generated file with path '" + std::string(suffix) + "'");
  return nullptr;
}

void test_cpp_backend_emits_structs_and_functions() {
  IRModule module("SampleModule");
  module.add_struct(
      "Pair",
      {IRField{.name = "first", .type = IRType::Generic("T")},
       IRField{.name = "second", .type = IRType::Generic("T")}},
      {"T"});

  auto& fn =
      module.add_function("add_values", IRType::Generic("T"),
                          {IRParameter{.name = "a", .type = IRType::Generic("T")},
                           IRParameter{.name = "b", .type = IRType::Generic("T")}},
                          {"T"});
  fn.add_instruction(IRValue{.result = "sum", .op = "add", .operands = {"a", "b"}});
  fn.add_instruction(IRValue{.op = "ret", .operands = {"sum"}});

  CppBackend backend{};
  TargetProfile profile{.name = "cpp20", .version = "20"};
  const auto files = backend.emit(module, profile);

  expect(files.size() == 2, "backend should emit header and source files");

  const auto* header = find_file(files, "samplemodule.hpp");
  expect(header != nullptr, "header file should be present");
  expect(header->contents.find("template <typename T>\nstruct Pair") != std::string::npos,
         "header should contain template struct definition");
  expect(header->contents.find("std::int32_t") == std::string::npos,
         "header should not introduce unused includes for unrelated types");
  expect(header->contents.find("add_values") != std::string::npos,
         "header should declare template function");
  expect(header->contents.find("namespace istudio::generated") != std::string::npos,
         "header should open generated namespace");

  const auto* source = find_file(files, "samplemodule.cpp");
  expect(source != nullptr, "source file should be present");
  expect(source->contents.find("#include \"samplemodule.hpp\"") != std::string::npos,
         "source should include generated header");
  expect(source->contents.find("auto sum = a + b;") != std::string::npos,
         "source should lower add instruction to arithmetic");
  expect(source->contents.find("return sum;") != std::string::npos,
         "source should emit return statement");
}

}  // namespace

void run_cpp_backend_tests() {
  test_cpp_backend_emits_structs_and_functions();
  std::cout << "All C++ backend tests passed\n";
}
