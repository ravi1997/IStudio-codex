#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "front/ast.h"
#include "sem/context.h"
#include "sem/types.h"
#include "support/diagnostics.h"

namespace istudio::sem {

class TypeTable {
 public:
  void set(front::NodeId id, Type type);
  [[nodiscard]] Type get(front::NodeId id) const;
  [[nodiscard]] bool contains(front::NodeId id) const noexcept;
  void clear();

 private:
  std::unordered_map<front::NodeId, Type> types_{};
};

class SemanticAnalyzer {
 public:
  SemanticAnalyzer(const front::AstContext& ast, support::DiagnosticReporter& reporter);

  void analyze(front::NodeId root);

  [[nodiscard]] const SemanticContext& context() const noexcept { return context_; }
  [[nodiscard]] const TypeTable& types() const noexcept { return types_; }

 private:
  void analyze_node(front::NodeId id);
  void analyze_module(const front::AstNode& node);
  void analyze_block(const front::AstNode& node);
  void analyze_function(const front::AstNode& node);
  void analyze_let(const front::AstNode& node);
  void analyze_return(const front::AstNode& node);
  void analyze_expression_statement(const front::AstNode& node);

  Type analyze_expression(front::NodeId id);
  Type analyze_identifier(const front::AstNode& node);
  Type analyze_literal(const front::AstNode& node);
  Type analyze_binary(const front::AstNode& node);
  Type analyze_assignment(const front::AstNode& node);
  Type analyze_unary(const front::AstNode& node);
  Type analyze_group(const front::AstNode& node);
  Type analyze_call(const front::AstNode& node);

  void declare_symbol(std::string_view name, front::NodeId id, support::Span span);
  void assign_type(front::NodeId id, Type type);
  void update_current_function_return(Type return_type, const front::AstNode& node);
  Type unify_types(Type lhs, Type rhs, support::Span span, std::string_view context);

  const front::AstContext& ast_;
  support::DiagnosticReporter& reporter_;
  SemanticContext context_{};
  TypeTable types_{};
  struct ActiveFunction {
    FunctionSignature* signature{nullptr};
    Type inferred_return{};
    bool saw_return{false};
  };
  [[nodiscard]] ActiveFunction* current_function() noexcept;
  std::vector<ActiveFunction> function_stack_{};
};

}  // namespace istudio::sem
