#pragma once

#include <string>

#include "front/ast.h"

namespace istudio::front {

struct AstDumpOptions {
  bool include_ids{true};
  bool include_spans{true};
};

[[nodiscard]] std::string dump_ast_text(const AstContext& context, NodeId root, const AstDumpOptions& options = {});
[[nodiscard]] std::string dump_ast_json(const AstContext& context, NodeId root, const AstDumpOptions& options = {});

}  // namespace istudio::front

