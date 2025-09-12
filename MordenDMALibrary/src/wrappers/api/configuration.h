#pragma once

namespace VMM {

std::expected<uint64_t, std::string> ConfigGet(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle, uint64_t opt);

bool ConfigSet(const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
               uint64_t opt, uint64_t val);

};  // namespace VMM