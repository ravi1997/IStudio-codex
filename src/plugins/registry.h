#pragma once

#include <string>
#include <vector>

#include "backends/backend.h"

namespace istudio::plugins {

class BackendRegistry {
 public:
  void register_backend(backends::BackendPtr backend);
  [[nodiscard]] const std::vector<backends::BackendPtr>& backends() const noexcept { return backends_; }

 private:
  std::vector<backends::BackendPtr> backends_{};
};

}  // namespace istudio::plugins
