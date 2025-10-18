#pragma once

#include <string>
#include <vector>

#include "backends/backend.h"

namespace istudio::backends::cpp {

struct CppBackendOptions {
  std::string namespace_name{"istudio::generated"};
  std::string header_suffix{".hpp"};
  std::string source_suffix{".cpp"};
  bool emit_header{true};
  bool emit_source{true};
};

class CppBackend : public Backend {
 public:
  explicit CppBackend(CppBackendOptions options = {});

  [[nodiscard]] std::string name() const override;
  [[nodiscard]] std::vector<GeneratedFile> emit(const ir::IRModule& module,
                                                const TargetProfile& profile) override;

 private:
  CppBackendOptions options_{};
};

}  // namespace istudio::backends::cpp

