#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>

#include "front/ast.h"
#include "ir/lowering.h"
#include "sem/analyzer.h"
#include "support/diagnostics.h"
#include "support/span.h"

using istudio::front::AstContext;
using istudio::front::AstKind;
using istudio::front::NodeId;
using istudio::ir::IRModule;
using istudio::ir::IRTypeKind;
using istudio::ir::lower_module;
using istudio::sem::SemanticAnalyzer;
using istudio::sem::TypeKind;
using istudio::support::DiagCode;
using istudio::support::DiagnosticReporter;
using istudio::support::Span;

namespace {

[[noreturn]] void fail(const std::string& message) {
  throw std::runtime_error(message);
}

void expect(bool condition, const std::string& message) {
  if (!condition) {
    fail(message);
  }
}

NodeId make_identifier(AstContext& ast, Span span, const std::string& value) {
  return ast.create_node(AstKind::IdentifierExpr, span, value).id;
}

NodeId make_literal(AstContext& ast, Span span, const std::string& value) {
  return ast.create_node(AstKind::LiteralExpr, span, value).id;
}

NodeId make_call(AstContext& ast, Span span, const std::string& callee_name,
                 const std::vector<NodeId>& args) {
  const NodeId callee_id = make_identifier(ast, span, callee_name);
  auto& call = ast.create_node(AstKind::CallExpr, span);
  call.children.push_back(callee_id);
  for (auto arg : args) {
    call.children.push_back(arg);
  }
  return call.id;
}

NodeId make_return_literal_function(AstContext& ast, Span span, const std::string& name,
                                    const std::vector<std::string>& params,
                                    const std::string& literal_value) {
  const NodeId name_id = make_identifier(ast, span, name);
  auto& param_list = ast.create_node(AstKind::ArgumentList, span);
  for (const auto& param : params) {
    param_list.children.push_back(make_identifier(ast, span, param));
  }

  const NodeId literal_id = make_literal(ast, span, literal_value);
  auto& return_stmt = ast.create_node(AstKind::ReturnStmt, span);
  return_stmt.children.push_back(literal_id);

  auto& body = ast.create_node(AstKind::BlockStmt, span);
  body.children.push_back(return_stmt.id);

  auto& function = ast.create_node(AstKind::Function, span);
  function.children.push_back(name_id);
  function.children.push_back(param_list.id);
  function.children.push_back(body.id);
  return function.id;
}

struct ModuleFixture {
  AstContext ast{};
  DiagnosticReporter reporter{};
  std::unique_ptr<SemanticAnalyzer> analyzer{};
  NodeId module_id{};
  NodeId primary_call_id{};
  NodeId mismatch_call_id{};
};

void build_module(ModuleFixture& fixture, bool include_mismatch_call) {
  const Span span{};

  fixture.module_id = fixture.ast.create_node(AstKind::Module, span).id;

  const NodeId function_id =
      make_return_literal_function(fixture.ast, span, "add", {"x", "y"}, "1");
  fixture.ast.node(fixture.module_id).children.push_back(function_id);

  const NodeId call_expr_id =
      make_call(fixture.ast, span, "add",
                {make_literal(fixture.ast, span, "1"), make_literal(fixture.ast, span, "2")});
  fixture.primary_call_id = call_expr_id;

  auto& let_stmt = fixture.ast.create_node(AstKind::LetStmt, span, "let");
  const NodeId result_id = make_identifier(fixture.ast, span, "result");
  let_stmt.children.push_back(result_id);
  let_stmt.children.push_back(call_expr_id);
  fixture.ast.node(fixture.module_id).children.push_back(let_stmt.id);

  if (include_mismatch_call) {
    const NodeId bad_call_id = make_call(
        fixture.ast, span, "add",
        {make_literal(fixture.ast, span, "\"oops\""), make_literal(fixture.ast, span, "3")});
    fixture.mismatch_call_id = bad_call_id;
    auto& expr_stmt = fixture.ast.create_node(AstKind::ExpressionStmt, span);
    expr_stmt.children.push_back(bad_call_id);
    fixture.ast.node(fixture.module_id).children.push_back(expr_stmt.id);
  }

  fixture.analyzer = std::make_unique<SemanticAnalyzer>(fixture.ast, fixture.reporter);
  fixture.analyzer->analyze(fixture.module_id);

  const auto& module_node = fixture.ast.node(fixture.module_id);
  expect(!module_node.children.empty(), "builder: module should have at least one child");
}

void test_lowering_produces_typed_function() {
  ModuleFixture fixture{};
  build_module(fixture, false);

  const auto& module_node = fixture.ast.node(fixture.module_id);
  expect(!module_node.children.empty(), "module should contain at least one child");
  expect(fixture.ast.node(module_node.children.front()).kind == AstKind::Function,
         "module first child should be function");
  const auto& registry = fixture.analyzer->context().functions().entries();
  expect(!registry.empty(), "function registry should contain entries");
  const auto* signature = fixture.analyzer->context().functions().lookup("add");
  expect(signature != nullptr, "function signature should be present");
  expect(signature->parameters.size() == 2, "function should have two parameters");

  const auto& types = fixture.analyzer->types();
  expect(types.get(fixture.primary_call_id).kind == TypeKind::Integer,
         "call expression should infer integer return type");

  IRModule module =
      lower_module(fixture.ast, *fixture.analyzer, fixture.module_id, "example");

  const auto& functions = module.functions();
  auto it = std::find_if(functions.begin(), functions.end(),
                         [](const auto& fn) { return fn.name == "add"; });
  expect(it != functions.end(), "lowered module should contain add function");
  expect(it->return_type.kind == IRTypeKind::I64,
         "add should lower to 64-bit integer return type");
  expect(it->parameters.size() == 2, "lowered function should have two parameters");
  expect(it->parameters[0].type.kind == IRTypeKind::I64,
         "first parameter should lower to 64-bit integer");
  expect(it->parameters[1].type.kind == IRTypeKind::I64,
         "second parameter should lower to 64-bit integer");
}

void test_call_type_mismatch_reports_diagnostic() {
  ModuleFixture fixture{};
  build_module(fixture, true);
  const auto& diagnostics = fixture.reporter.diagnostics();
  expect(!diagnostics.empty(), "mismatched call should emit diagnostic");
  const bool has_type_mismatch = std::any_of(
      diagnostics.begin(), diagnostics.end(),
      [](const auto& diag) { return diag.code == DiagCode::SemTypeMismatch; });
  expect(has_type_mismatch, "expected SemTypeMismatch for argument mismatch");
}

}  // namespace

void run_ir_lowering_tests() {
  test_lowering_produces_typed_function();
  test_call_type_mismatch_reports_diagnostic();
}
