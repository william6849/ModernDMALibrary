#include "configuration.h"

namespace VMM {

std::expected<uint64_t, std::string> ConfigGet(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle, uint64_t opt) {
  uint64_t val = 0;

  if (!handle || !handle->Valid()) {
    return std::unexpected("Invalid handle");
  }

  auto result = VMMDLL_ConfigGet(handle->Get(), opt, &val);
  if (!result) {
    return std::unexpected(
        "VMMDLL_ConfigGet failed (opt: " + std::to_string(opt) + ")");
  }

  return val;
}

std::expected<void, std::string> ConfigSet(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle, uint64_t opt,
    uint64_t val) {
  if (!handle || !handle->get()) return std::unexpected("Invalid handle");

  if (!VMMDLL_ConfigSet(handle->get(), opt, val))
    return std::unexpected(
        "VMMDLL_ConfigSet failed (opt: " + std::to_string(opt) +
        ", val: " + std::to_string(val) + ")");

  return {};
}

};  // namespace VMM
