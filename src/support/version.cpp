#include "support/version.h"

#ifndef ISTUDIO_VERSION
#define ISTUDIO_VERSION "0.0.0"
#endif

namespace istudio::support {

std::string_view current_version() {
  return ISTUDIO_VERSION;
}

}  // namespace istudio::support
