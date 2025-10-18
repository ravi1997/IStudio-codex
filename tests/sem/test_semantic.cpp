#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>

#include "front/lexer.h"
#include "front/parser.h"
#include "sem/analyzer.h"
#include "support/diagnostics.h"
#include "support/span.h"

using istudio::front::AstContext;
using istudio::front::AstKind;
using istudio::front::LexerConfig;
using istudio::front::NodeId;
using istudio::front::lex;
using istudio::front::parse_module;
using istudio::sem::SemanticAnalyzer;
using istudio::sem::TypeKind;
using istudio::sem::Type;
using istudio::sem::TypeTable;
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

struct AnalysisContext {
  AstContext ast{};
  NodeId root{};
  DiagnosticReporter reporter{};
  std::unique_ptr<SemanticAnalyzer> analyzer{};
};

AnalysisContext analyze_source(const std::string& source) {
  AnalysisContext ctx{};
  LexerConfig config{};
  const auto tokens = lex(source, config);
  ctx.root = parse_module(tokens, ctx.ast);
  ctx.analyzer = std::make_unique<SemanticAnalyzer>(ctx.ast, ctx.reporter);
  ctx.analyzer->analyze(ctx.root);
  return ctx;
}

void test_duplicate_symbol_detection() {
  const auto ctx = analyze_source("let x = 1;\nlet x = 2;");
  const auto& diagnostics = ctx.reporter.diagnostics();
  expect(!diagnostics.empty(), "expected duplicate symbol diagnostic");
  expect(diagnostics.front().code == DiagCode::SemDuplicateSymbol, "expected SemDuplicateSymbol code");
}

void test_unknown_identifier_reports_error() {
  const auto ctx = analyze_source("return y;");
  const auto& diagnostics = ctx.reporter.diagnostics();
  expect(!diagnostics.empty(), "expected unknown identifier diagnostic");
  expect(diagnostics.front().code == DiagCode::SemUnknownIdentifier, "expected SemUnknownIdentifier code");
}

void test_integer_type_inference() {
  auto ctx = analyze_source("let x = 1;\nreturn x;");
  const auto& ast = ctx.ast;
  const auto& module = ast.node(ctx.root);
  expect(!module.children.empty(), "module should contain statements");
  const NodeId let_stmt_id = module.children.front();
  const auto& let_node = ast.node(let_stmt_id);
  expect(let_node.kind == AstKind::LetStmt, "first statement should be let");
  expect(let_node.children.size() >= 1, "let should have identifier child");
  const NodeId ident_id = let_node.children.front();
  const TypeKind ident_type = ctx.analyzer->types().get(ident_id).kind;
  expect(ident_type == TypeKind::Integer, "identifier should infer integer type");
}

void test_binary_type_mismatch_produces_diagnostic() {
  const auto ctx = analyze_source("let x = \"hi\";\nlet y = x + 1;");
  const auto& diagnostics = ctx.reporter.diagnostics();
  expect(!diagnostics.empty(), "expected type mismatch diagnostic");
  const bool has_type_mismatch = std::any_of(
      diagnostics.begin(), diagnostics.end(),
      [](const auto& diag) { return diag.code == DiagCode::SemTypeMismatch; });
  expect(has_type_mismatch, "expected SemTypeMismatch diagnostic");
}

void test_function_signature_recording() {
  AnalysisContext ctx{};
  const Span span{};

  const NodeId func_name_id = ctx.ast.create_node(AstKind::IdentifierExpr, span, "add").id;
  const NodeId param_list_id = ctx.ast.create_node(AstKind::ArgumentList, span).id;
  const NodeId param_x_id = ctx.ast.create_node(AstKind::IdentifierExpr, span, "x").id;
  const NodeId param_y_id = ctx.ast.create_node(AstKind::IdentifierExpr, span, "y").id;
  ctx.ast.node(param_list_id).children.push_back(param_x_id);
  ctx.ast.node(param_list_id).children.push_back(param_y_id);

  const NodeId literal_id = ctx.ast.create_node(AstKind::LiteralExpr, span, "1").id;
  const NodeId return_id = ctx.ast.create_node(AstKind::ReturnStmt, span).id;
  ctx.ast.node(return_id).children.push_back(literal_id);

  const NodeId body_id = ctx.ast.create_node(AstKind::BlockStmt, span).id;
  ctx.ast.node(body_id).children.push_back(return_id);

  const NodeId function_id = ctx.ast.create_node(AstKind::Function, span).id;
  auto& function = ctx.ast.node(function_id);
  function.children.push_back(func_name_id);
  function.children.push_back(param_list_id);
  function.children.push_back(body_id);

  expect(function.children.size() == 3, "function should reference name, params, and body");
  expect(function.children[0] == func_name_id, "function child[0] should be name identifier");
  expect(function.children[1] == param_list_id, "function child[1] should be parameter list");
  expect(function.children[2] == body_id, "function child[2] should be body block");

  ctx.root = function_id;
  ctx.analyzer = std::make_unique<SemanticAnalyzer>(ctx.ast, ctx.reporter);
  ctx.analyzer->analyze(ctx.root);

  const Type name_type = ctx.analyzer->types().get(func_name_id);
  expect(name_type.kind == TypeKind::Function, "function identifier should be typed as function");

  const auto* signature = ctx.analyzer->context().functions().lookup("add");
  expect(signature != nullptr, "expected function signature to be recorded");
  expect(signature->parameters.size() == 2, "expected two parameters");
  expect(signature->parameters[0].name == "x", "expected first parameter to be x");
  expect(signature->parameters[1].name == "y", "expected second parameter to be y");
  expect(signature->return_type.kind == TypeKind::Integer, "expected integer return type inference");

  const auto* signature_by_id = ctx.analyzer->context().functions().lookup(function_id);
  expect(signature_by_id == signature, "lookup by node id should match signature by name");

  const NodeId symbol_id = ctx.analyzer->context().symbols().lookup("add");
  expect(symbol_id == func_name_id, "function name should be present in symbol table");
}

void test_call_expression_infers_return_type() {
  AnalysisContext ctx{};
  const Span span{};

  const NodeId func_name_id = ctx.ast.create_node(AstKind::IdentifierExpr, span, "add").id;
  const NodeId param_list_id = ctx.ast.create_node(AstKind::ArgumentList, span).id;
  const NodeId param_x_id = ctx.ast.create_node(AstKind::IdentifierExpr, span, "x").id;
  const NodeId param_y_id = ctx.ast.create_node(AstKind::IdentifierExpr, span, "y").id;
  ctx.ast.node(param_list_id).children.push_back(param_x_id);
  ctx.ast.node(param_list_id).children.push_back(param_y_id);

  const NodeId literal_id = ctx.ast.create_node(AstKind::LiteralExpr, span, "1").id;
  const NodeId return_id = ctx.ast.create_node(AstKind::ReturnStmt, span).id;
  ctx.ast.node(return_id).children.push_back(literal_id);

  const NodeId body_id = ctx.ast.create_node(AstKind::BlockStmt, span).id;
  ctx.ast.node(body_id).children.push_back(return_id);

  const NodeId function_id = ctx.ast.create_node(AstKind::Function, span).id;
  auto& function_node = ctx.ast.node(function_id);
  function_node.children.push_back(func_name_id);
  function_node.children.push_back(param_list_id);
  function_node.children.push_back(body_id);

  const NodeId call_callee_id = ctx.ast.create_node(AstKind::IdentifierExpr, span, "add").id;
  const NodeId call_expr_id = ctx.ast.create_node(AstKind::CallExpr, span).id;
  ctx.ast.node(call_expr_id).children.push_back(call_callee_id);
  const NodeId call_stmt_id = ctx.ast.create_node(AstKind::ExpressionStmt, span).id;
  ctx.ast.node(call_stmt_id).children.push_back(call_expr_id);

  const NodeId block_id = ctx.ast.create_node(AstKind::BlockStmt, span).id;
  auto& block = ctx.ast.node(block_id);
  block.children.push_back(function_id);
  block.children.push_back(call_stmt_id);

  ctx.root = block_id;

  ctx.analyzer = std::make_unique<SemanticAnalyzer>(ctx.ast, ctx.reporter);
  ctx.analyzer->analyze(ctx.root);

  const Type call_type = ctx.analyzer->types().get(call_expr_id);
  expect(call_type.kind == TypeKind::Integer, "call expression should inherit function return type");

  const Type callee_type = ctx.analyzer->types().get(call_callee_id);
  expect(callee_type.kind == TypeKind::Function, "callee should be typed as function");
  expect(callee_type.reference == function_id, "callee reference should point to function node");

  const Type stmt_type = ctx.analyzer->types().get(call_stmt_id);
  expect(stmt_type.kind == TypeKind::Integer, "expression statement should carry call result type");
}

void test_conflicting_return_types_report_error() {
  AnalysisContext ctx{};
  const Span span{};
  const NodeId func_name_id = ctx.ast.create_node(AstKind::IdentifierExpr, span, "mix").id;
  const NodeId body_id = ctx.ast.create_node(AstKind::BlockStmt, span).id;

  const NodeId int_literal_id = ctx.ast.create_node(AstKind::LiteralExpr, span, "1").id;
  const NodeId first_return_id = ctx.ast.create_node(AstKind::ReturnStmt, span).id;
  ctx.ast.node(first_return_id).children.push_back(int_literal_id);

  const NodeId str_literal_id = ctx.ast.create_node(AstKind::LiteralExpr, span, "\"two\"").id;
  const NodeId second_return_id = ctx.ast.create_node(AstKind::ReturnStmt, span).id;
  ctx.ast.node(second_return_id).children.push_back(str_literal_id);

  auto& body = ctx.ast.node(body_id);
  body.children.push_back(first_return_id);
  body.children.push_back(second_return_id);

  const NodeId function_id = ctx.ast.create_node(AstKind::Function, span).id;
  auto& function = ctx.ast.node(function_id);
  function.children.push_back(func_name_id);
  function.children.push_back(body_id);

  ctx.root = function_id;
  ctx.analyzer = std::make_unique<SemanticAnalyzer>(ctx.ast, ctx.reporter);
  ctx.analyzer->analyze(ctx.root);

  const auto& diagnostics = ctx.reporter.diagnostics();
  expect(!diagnostics.empty(), "expected diagnostic for conflicting returns");
  expect(diagnostics.front().code == DiagCode::SemTypeMismatch, "expected SemTypeMismatch for conflicting returns");

  const auto* signature = ctx.analyzer->context().functions().lookup("mix");
  expect(signature != nullptr, "expected signature for mix");
  const auto actual_kind = signature->return_type.kind;
  expect(actual_kind == TypeKind::Unknown,
         "conflicting returns should mark return type as unknown (actual kind=" +
             std::to_string(static_cast<int>(actual_kind)) + ")");
}

}  // namespace

void run_semantic_tests() {
  test_duplicate_symbol_detection();
  test_unknown_identifier_reports_error();
  test_integer_type_inference();
  test_binary_type_mismatch_produces_diagnostic();
  test_function_signature_recording();
  test_call_expression_infers_return_type();
  test_conflicting_return_types_report_error();
}
