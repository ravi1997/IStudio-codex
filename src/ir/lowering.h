#pragma once

#include <string>

#include "front/ast.h"
#include "ir/module.h"
#include "sem/analyzer.h"

namespace istudio::ir {

IRModule lower_module(const front::AstContext& ast, const sem::SemanticAnalyzer& analyzer,
                      front::NodeId root, std::string module_name = "module");

}

