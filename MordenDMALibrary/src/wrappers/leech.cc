#include "leech.h"

#include "spdlog/spdlog.h"

namespace LC {
void HandleDeleter(HANDLE handle) {
  if (handle) {
    spdlog::debug("Closing handler: {}", static_cast<void*>(handle));
    LcClose(handle);
  }
}
}  // namespace LC

namespace VMM {
void HandleDeleter(VMM_HANDLE handle) {
  if (handle) {
    spdlog::debug("Closing VMM handler: {}", static_cast<void*>(handle));
    VMMDLL_Close(handle);
  }
}

}  // namespace VMM
