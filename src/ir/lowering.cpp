#include "ir/lowering.h"

#include <string>

#include "sem/context.h"
#include "sem/types.h"

namespace istudio::ir {
namespace {

IRType map_type(const sem::Type& type) {
  using sem::TypeKind;
  switch (type.kind) {
    case TypeKind::Void:
      return IRType::Void();
    case TypeKind::Integer:
      return IRType::I64();
    case TypeKind::Float:
      return IRType::F64();
    case TypeKind::Bool:
      return IRType::Bool();
    case TypeKind::String:
      return IRType::String();
    case TypeKind::Function:
      return IRType::Generic("fn");
    case TypeKind::Unknown:
    default:
      return IRType::Void();
  }
}

}  // namespace

IRModule lower_module(const front::AstContext&, const sem::SemanticAnalyzer& analyzer,
                      front::NodeId, std::string module_name) {
  IRModule module(std::move(module_name));

  const auto& registry = analyzer.context().functions().entries();
  for (const auto& [name, signature] : registry) {
    std::vector<IRParameter> params;
    params.reserve(signature.parameters.size());
    for (const auto& param : signature.parameters) {
      params.push_back(IRParameter{param.name, map_type(param.type)});
    }

    IRType return_type = map_type(signature.return_type);
    module.add_function(name, return_type, std::move(params));
  }

  return module;
}

}  // namespace istudio::ir

