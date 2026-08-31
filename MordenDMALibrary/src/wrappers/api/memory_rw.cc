#include "memory_rw.h"

#include <ranges>

namespace VMM {

std::expected<std::vector<uint8_t>, std::string> MemReadPage(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const uint64_t addr) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle.");
  }

  auto out_buffer = std::array<uint8_t, DEFAULT_PAGE_BYTES>;
  auto result = VMMDLL_MemReadPage(handle->get(), static_cast<DWORD>(pid),
                                   static_cast<ULONG64>(addr),
                                   static_cast<PBYTE>(out_buffer.data()));

  if (!result) {
    std::stringstream err;
    err << "VMMDLL_MemReadPage failed. pid=" << pid << ", addr=0x" << std::hex
        << addr;
    return std::unexpected(err.str());
  }

  return out_buffer;
}

std::expected<std::vector<uint8_t>, std::string> MemReadEx(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const uint64_t addr, const std::size_t bytes,
    const uint32_t flag) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  if (bytes == 0) {
    return std::unexpected("Bytes to read must be greater than zero");
  }

  auto ret = std::vector<uint8_t>(bytes);
  DWORD read_bytes = 0;
  auto result = VMMDLL_MemReadEx(
      handle->get(), static_cast<DWORD>(pid), static_cast<ULONG64>(addr),
      static_cast<PBYTE>(ret.data()), static_cast<DWORD>(bytes), &read_bytes,
      static_cast<DWORD>(flag));

  if (!result) {
    return std::unexpected("VMMDLL_MemReadEx call failed");
  }

  if (read_bytes != bytes) {
    return std::unexpected("VMMDLL_MemReadEx incomplete read: requested " +
                           std::to_string(bytes) + " bytes, but got " +
                           std::to_string(read_bytes));
  }

  return ret;
}

std::expected<bool, std::string> MemPrefetchPages(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const std::vector<uint64_t>& prefetch_addresses) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  if (prefetch_addresses.empty()) {
    return std::unexpected("Prefetch addresses vector is empty");
  }

  auto vec_ull = prefetch_addresses | std::views::transform([](uint64_t addr) {
                   return static_cast<unsigned long long>(addr);
                 }) |
                 std::ranges::to<std::vector>();

  bool result = VMMDLL_MemPrefetchPages(handle->get(), static_cast<DWORD>(pid),
                                        vec_ull.data(), vec_ull.size());

  if (!result) {
    return std::unexpected("VMMDLL_MemPrefetchPages failed");
  }

  return true;
}

std::expected<void, std::string> MemWrite(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const uint64_t addr, const std::vector<uint8_t>& data) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  if (data.empty()) {
    return std::unexpected("Data buffer is empty");
  }

  bool result = VMMDLL_MemWrite(
      handle->get(), static_cast<DWORD>(pid), static_cast<ULONG64>(addr),
      static_cast<PBYTE>(data.data()), static_cast<DWORD>(data.size()));

  if (!result) {
    return std::unexpected("VMMDLL_MemWrite failed");
  }

  return {};
}

std::expected<uint64_t, std::string> MemVirt2Phys(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const uint64_t addr) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  ULONG64 result = 0;
  bool ret = VMMDLL_MemVirt2Phys(handle->get(), static_cast<DWORD>(pid),
                                 static_cast<ULONG64>(addr), &result);
  if (!ret) {
    return std::unexpected("VMMDLL_MemVirt2Phys failed");
  }
  return result;
}

std::expected<std::vector<uint64_t>, std::string> MemSearch(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const std::shared_ptr<MemorySearchContext>& context) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }
  if (!context) {
    return std::unexpected("Invalid MemorySearchContext");
  }

  DWORD search_result_count = 0;
  PQWORD search_result_ptr = nullptr;

  bool ret = VMMDLL_MemSearch(handle->get(), static_cast<DWORD>(pid),
                              &context->raw_context, &search_result_ptr,
                              &search_result_count);

  if (!ret) {
    return std::unexpected("VMMDLL_MemSearch failed");
  }

  auto result = std::vector<uint64_t>{};
  if (search_result_ptr != nullptr && search_result_count > 0) {
    result.reserve(search_result_count);
    for (DWORD i = 0; i < search_result_count; ++i) {
      result.push_back(search_result_ptr[i]);
    }
    VMMDLL_MemFree(search_result_ptr);
  }

  return result;
}

std::expected<uint32_t, std::string> MemReadScatter(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, PPMEM_SCATTER scatter_list, uint64_t scatter_size,
    uint32_t flags) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  if (scatter_list == nullptr) {
    return std::unexpected("scatter_list is null");
  }

  if (scatter_size == 0) {
    return std::unexpected("scatter_size is zero");
  }

  uint32_t ret = VMMDLL_MemReadScatter(handle->get(), pid, scatter_list,
                                       static_cast<DWORD>(scatter_size), flags);

  if (ret == 0) {
    return std::unexpected("VMMDLL_MemReadScatter failed");
  }

  return ret;
}

std::expected<uint32_t, std::string> MemWriteScatter(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, PPMEM_SCATTER scatter_list, uint64_t scatter_size) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  if (scatter_list == nullptr) {
    return std::unexpected("scatter_list is null");
  }

  if (scatter_size == 0) {
    return std::unexpected("scatter_size is zero");
  }

  uint32_t ret = VMMDLL_MemWriteScatter(handle->get(), pid, scatter_list,
                                        static_cast<DWORD>(scatter_size));

  if (ret == 0) {
    return std::unexpected("VMMDLL_MemWriteScatter failed");
  }

  return ret;
}

};  // namespace VMM
