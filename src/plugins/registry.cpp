#include "plugins/registry.h"

#include <utility>

namespace istudio::plugins {

void BackendRegistry::register_backend(backends::BackendPtr backend) {
  backends_.push_back(std::move(backend));
}

}  // namespace istudio::plugins
