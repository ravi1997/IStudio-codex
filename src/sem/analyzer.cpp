#include "sem/analyzer.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

namespace istudio::sem {
namespace {

constexpr front::NodeId kInvalidNode = std::numeric_limits<front::NodeId>::max();

bool is_bool_literal(std::string_view value) {
  return value == "true" || value == "false";
}

bool is_number_literal(std::string_view value) {
  if (value.empty()) {
    return false;
  }
  bool seen_decimal = false;
  for (char ch : value) {
    if (std::isdigit(static_cast<unsigned char>(ch)) != 0) {
      continue;
    }
    if (ch == '.' && !seen_decimal) {
      seen_decimal = true;
      continue;
    }
    return false;
  }
  return true;
}

bool is_float_literal(std::string_view value) {
  return value.find('.') != std::string_view::npos;
}

inline Type pick_known(Type lhs, Type rhs) {
  if (lhs.kind != TypeKind::Unknown) {
    return lhs;
  }
  return rhs;
}

}  // namespace

void TypeTable::set(front::NodeId id, Type type) {
  types_[id] = type;
}

Type TypeTable::get(front::NodeId id) const {
  auto it = types_.find(id);
  if (it != types_.end()) {
    return it->second;
  }
  return Type{};
}

bool TypeTable::contains(front::NodeId id) const noexcept {
  return types_.find(id) != types_.end();
}

void TypeTable::clear() {
  types_.clear();
}

SemanticAnalyzer::SemanticAnalyzer(const front::AstContext& ast, support::DiagnosticReporter& reporter)
    : ast_(ast), reporter_(reporter) {}

void SemanticAnalyzer::analyze(front::NodeId root) {
  types_.clear();
  context_ = SemanticContext{};
  analyze_node(root);
}

void SemanticAnalyzer::analyze_node(front::NodeId id) {
  const auto& node = ast_.node(id);
  switch (node.kind) {
    case front::AstKind::Module:
      analyze_module(node);
      break;
    case front::AstKind::Function:
      analyze_function(node);
      break;
    case front::AstKind::BlockStmt:
      analyze_block(node);
      break;
    case front::AstKind::LetStmt:
      analyze_let(node);
      break;
    case front::AstKind::ReturnStmt:
      analyze_return(node);
      break;
    case front::AstKind::ExpressionStmt:
      analyze_expression_statement(node);
      break;
    default:
      break;
  }
}

void SemanticAnalyzer::analyze_module(const front::AstNode& node) {
  for (front::NodeId child : node.children) {
    analyze_node(child);
  }
  assign_type(node.id, Type{TypeKind::Unknown});
}

void SemanticAnalyzer::analyze_block(const front::AstNode& node) {
  context_.symbols().push_scope();
  for (front::NodeId child : node.children) {
    analyze_node(child);
  }
  context_.symbols().pop_scope();
  assign_type(node.id, Type{TypeKind::Unknown});
}

void SemanticAnalyzer::analyze_function(const front::AstNode& node) {
  if (node.children.empty()) {
    Type function_type{TypeKind::Function, node.id};
    assign_type(node.id, function_type);
    return;
  }

  const auto& name_node = ast_.node(node.children.front());
  declare_symbol(name_node.value, name_node.id, name_node.span);

  Type function_type{TypeKind::Function, node.id};
  assign_type(name_node.id, function_type);
  assign_type(node.id, function_type);

  FunctionSignature signature{};
  signature.name = name_node.value;
  signature.node_id = node.id;
  signature.return_type = Type{TypeKind::Unknown};

  std::size_t next_index = 1;
  if (node.children.size() > 1) {
    const auto& potential_params = ast_.node(node.children[1]);
    if (potential_params.kind == front::AstKind::ArgumentList) {
      for (front::NodeId param_id : potential_params.children) {
        const auto& param_node = ast_.node(param_id);
        FunctionParameter param{};
        param.name = param_node.value;
        param.node_id = param_node.id;
        param.type = Type{TypeKind::Unknown};
        signature.parameters.push_back(std::move(param));
      }
      next_index = 2;
    }
  }

  auto [entry, inserted] = context_.functions().declare(std::move(signature));
  if (!inserted) {
    reporter_.report(support::DiagCode::SemDuplicateSymbol,
                     "duplicate function '" + name_node.value + "'", name_node.span);
  }

  function_stack_.push_back(
      ActiveFunction{.signature = entry, .inferred_return = Type{TypeKind::Unknown}, .saw_return = false});

  context_.symbols().push_scope();
  if (entry != nullptr) {
    for (auto& param : entry->parameters) {
      const auto& param_node = ast_.node(param.node_id);
      declare_symbol(param.name, param.node_id, param_node.span);
      assign_type(param.node_id, param.type);
    }
  }

  for (std::size_t i = next_index; i < node.children.size(); ++i) {
    analyze_node(node.children[i]);
  }

  context_.symbols().pop_scope();

  ActiveFunction active = function_stack_.back();
  function_stack_.pop_back();

  if (entry != nullptr) {
    Type return_type = active.inferred_return;
    if (!active.saw_return && return_type.kind == TypeKind::Unknown) {
      return_type.kind = TypeKind::Void;
    }
    entry->return_type = return_type;
    for (auto& param : entry->parameters) {
      param.type = types_.get(param.node_id);
    }
  }
}

void SemanticAnalyzer::analyze_let(const front::AstNode& node) {
  if (node.children.empty()) {
    assign_type(node.id, Type{TypeKind::Unknown});
    return;
  }

  const auto& name_node = ast_.node(node.children[0]);
  declare_symbol(name_node.value, name_node.id, name_node.span);

  Type init_type{TypeKind::Unknown};
  if (node.children.size() > 1) {
    init_type = analyze_expression(node.children[1]);
  }

  assign_type(name_node.id, init_type);
  assign_type(node.id, init_type);
}

void SemanticAnalyzer::analyze_return(const front::AstNode& node) {
  Type return_type{TypeKind::Void};
  if (!node.children.empty()) {
    return_type = analyze_expression(node.children.front());
  }
  assign_type(node.id, return_type);

  if (ActiveFunction* active = current_function()) {
    if (active->signature != nullptr) {
      std::string message =
          "return type mismatch for function '" + active->signature->name + "'";
      Type unified = unify_types(active->signature->return_type, return_type, node.span, message);
      active->signature->return_type = unified;
      return_type = unified;
    }
  }
  update_current_function_return(return_type, node);
}

void SemanticAnalyzer::analyze_expression_statement(const front::AstNode& node) {
  if (!node.children.empty()) {
    const Type expr_type = analyze_expression(node.children.front());
    assign_type(node.id, expr_type);
  } else {
    assign_type(node.id, Type{TypeKind::Unknown});
  }
}

Type SemanticAnalyzer::analyze_expression(front::NodeId id) {
  const auto& node = ast_.node(id);
  switch (node.kind) {
    case front::AstKind::IdentifierExpr:
      return analyze_identifier(node);
    case front::AstKind::LiteralExpr:
      return analyze_literal(node);
    case front::AstKind::BinaryExpr:
      return analyze_binary(node);
    case front::AstKind::AssignmentExpr:
      return analyze_assignment(node);
    case front::AstKind::UnaryExpr:
      return analyze_unary(node);
    case front::AstKind::GroupExpr:
      return analyze_group(node);
    case front::AstKind::CallExpr:
      return analyze_call(node);
    default:
      for (front::NodeId child : node.children) {
        analyze_expression(child);
      }
      Type result{TypeKind::Unknown};
      assign_type(node.id, result);
      return result;
  }
}

Type SemanticAnalyzer::analyze_identifier(const front::AstNode& node) {
  const front::NodeId symbol_id = context_.symbols().lookup(node.value);
  if (symbol_id == kInvalidNode) {
    reporter_.report(support::DiagCode::SemUnknownIdentifier,
                     "use of undeclared symbol '" + node.value + "'", node.span);
    Type type{TypeKind::Unknown};
    assign_type(node.id, type);
    return type;
  }

  const Type decl_type = types_.get(symbol_id);
  assign_type(node.id, decl_type);
  return decl_type;
}

Type SemanticAnalyzer::analyze_literal(const front::AstNode& node) {
  Type result{TypeKind::Unknown};
  const std::string& value = node.value;

  if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
    result.kind = TypeKind::String;
  } else if (is_bool_literal(value)) {
    result.kind = TypeKind::Bool;
  } else if (is_number_literal(value)) {
    result.kind = is_float_literal(value) ? TypeKind::Float : TypeKind::Integer;
  }

  assign_type(node.id, result);
  return result;
}

Type SemanticAnalyzer::analyze_binary(const front::AstNode& node) {
  if (node.children.size() < 2) {
    Type result{TypeKind::Unknown};
    assign_type(node.id, result);
    return result;
  }

  const Type left = analyze_expression(node.children[0]);
  const Type right = analyze_expression(node.children[1]);
  std::string message = "type mismatch in '" + node.value + "' expression";
  Type result = unify_types(left, right, node.span, message);
  assign_type(node.id, result);
  return result;
}

Type SemanticAnalyzer::analyze_assignment(const front::AstNode& node) {
  if (node.children.size() < 2) {
    Type result{TypeKind::Unknown};
    assign_type(node.id, result);
    return result;
  }

  const front::NodeId lhs_id = node.children[0];
  const front::NodeId rhs_id = node.children[1];
  Type left = analyze_expression(lhs_id);
  const Type right = analyze_expression(rhs_id);
  Type result = unify_types(left, right, node.span, "type mismatch in assignment");

  const auto& lhs_node = ast_.node(lhs_id);
  if (lhs_node.kind == front::AstKind::IdentifierExpr) {
    const front::NodeId decl_id = context_.symbols().lookup(lhs_node.value);
    if (decl_id != kInvalidNode) {
      Type decl_type = types_.get(decl_id);
      Type unified = unify_types(decl_type, right, lhs_node.span,
                                 "assignment to '" + lhs_node.value + "'");
      types_.set(decl_id, unified);
      assign_type(lhs_id, unified);
      left = unified;
    }
  }

  result = pick_known(right, left);
  assign_type(node.id, result);
  return result;
}

Type SemanticAnalyzer::analyze_unary(const front::AstNode& node) {
  if (node.children.empty()) {
    Type result{TypeKind::Unknown};
    assign_type(node.id, result);
    return result;
  }
  const Type operand = analyze_expression(node.children.front());
  assign_type(node.id, operand);
  return operand;
}

Type SemanticAnalyzer::analyze_group(const front::AstNode& node) {
  if (node.children.empty()) {
    Type result{TypeKind::Unknown};
    assign_type(node.id, result);
    return result;
  }
  const Type inner = analyze_expression(node.children.front());
  assign_type(node.id, inner);
  return inner;
}

Type SemanticAnalyzer::analyze_call(const front::AstNode& node) {
  if (node.children.empty()) {
    Type result{TypeKind::Unknown};
    assign_type(node.id, result);
    return result;
  }

  const front::NodeId callee_id = node.children.front();
  const Type callee_type = analyze_expression(callee_id);
  std::vector<Type> argument_types;
  argument_types.reserve(node.children.size() > 0 ? node.children.size() - 1 : 0);
  for (std::size_t i = 1; i < node.children.size(); ++i) {
    argument_types.push_back(analyze_expression(node.children[i]));
  }

  Type result{TypeKind::Unknown};
  if (callee_type.kind == TypeKind::Function) {
    if (auto* signature = context_.functions().lookup(callee_type.reference)) {
      const std::size_t expected_params = signature->parameters.size();
      const std::size_t provided_args = argument_types.size();
      if (expected_params != provided_args) {
        reporter_.report(support::DiagCode::SemArgumentCountMismatch,
                         "expected " + std::to_string(expected_params) +
                             " argument(s) but got " + std::to_string(provided_args) +
                             " when calling '" + signature->name + "'",
                         node.span);
      }

      const std::size_t limit = std::min(expected_params, provided_args);
      for (std::size_t i = 0; i < limit; ++i) {
        const auto& param = signature->parameters[i];
        Type param_type = types_.get(param.node_id);
        const auto& arg_node = ast_.node(node.children[1 + i]);
        std::string message =
            "argument type mismatch for parameter '" + param.name + "'";
        Type unified = unify_types(param_type, argument_types[i], arg_node.span, message);
        types_.set(param.node_id, unified);
        signature->parameters[i].type = unified;
      }

      result = signature->return_type;
    }
  }

  assign_type(node.id, result);
  return result;
}

void SemanticAnalyzer::declare_symbol(std::string_view name, front::NodeId id, support::Span span) {
  std::string name_copy{name};
  if (!context_.symbols().insert(name_copy, id)) {
    reporter_.report(support::DiagCode::SemDuplicateSymbol, "duplicate symbol '" + name_copy + "'", span);
  }
}

void SemanticAnalyzer::assign_type(front::NodeId id, Type type) {
  types_.set(id, type);
}

void SemanticAnalyzer::update_current_function_return(Type return_type, const front::AstNode& node) {
  if (function_stack_.empty()) {
    return;
  }

  ActiveFunction& active = function_stack_.back();
  if (return_type.kind != TypeKind::Void) {
    active.saw_return = true;
  }

  if (return_type.kind == TypeKind::Unknown) {
    active.inferred_return = Type{TypeKind::Unknown};
    if (active.signature != nullptr) {
      active.signature->return_type = Type{TypeKind::Unknown};
    }
    return;
  }

  std::string conflict_message = "conflicting return types";
  if (active.signature != nullptr) {
    conflict_message += " in function '" + active.signature->name + "'";
  }

  active.inferred_return =
      unify_types(active.inferred_return, return_type, node.span, conflict_message);

  if (active.signature != nullptr) {
    active.signature->return_type = active.inferred_return;
  }
}

Type SemanticAnalyzer::unify_types(Type lhs, Type rhs, support::Span span, std::string_view context) {
  if (lhs.kind == TypeKind::Unknown) {
    return rhs;
  }
  if (rhs.kind == TypeKind::Unknown) {
    return lhs;
  }

  if (lhs.kind == rhs.kind) {
    if (lhs.kind == TypeKind::Function && lhs.reference != rhs.reference) {
      reporter_.report(support::DiagCode::SemTypeMismatch, std::string(context), span);
      return Type{TypeKind::Unknown};
    }
    return lhs;
  }

  reporter_.report(support::DiagCode::SemTypeMismatch, std::string(context), span);
  return Type{TypeKind::Unknown};
}

SemanticAnalyzer::ActiveFunction* SemanticAnalyzer::current_function() noexcept {
  if (function_stack_.empty()) {
    return nullptr;
  }
  return &function_stack_.back();
}

}  // namespace istudio::sem
