#pragma once

namespace VMM {
struct MemorySearchContext {
  std::vector<VMMDLL_MEM_SEARCH_CONTEXT_SEARCHENTRY> search_entry{3};
  std::vector<std::vector<uint8_t>> pattern_buffer{
      3, {VMMDLL_MEM_SEARCH_MAXLENGTH}};
  VMMDLL_MEM_SEARCH_CONTEXT raw_context{.dwVersion = VMMDLL_MEM_SEARCH_VERSION,
                                        .cMaxResult = 0x10000,
                                        .vaMax = 0};

  MemorySearchContext(const std::vector<std::vector<uint8_t>>& targets,
                      const std::vector<std::vector<uint8_t>>& masks,
                      uint64_t min_virtual_address = 0,
                      uint64_t read_flags = VMMDLL_FLAG_NOCACHE,
                      uint64_t allignment = DEFAULT_PAGE_BYTES) {
    if (targets.empty() || masks.empty() ||
        (targets.at(0).size() != masks.at(0).size())) {
      spdlog::error("Empty bytes");
    }
    if (targets.at(0).size() > VMMDLL_MEM_SEARCH_MAXLENGTH) {
      spdlog::error("Search length exceed");
    }

    search_entry.resize(targets.size());
    raw_context.pSearch = search_entry.data();
    for (uint32_t entry_num = 0; entry_num < targets.size(); entry_num++) {
      search_entry.at(entry_num).cb = targets.at(entry_num).size();

      std::memcpy(search_entry.at(entry_num).pb,
                  pattern_buffer.at(entry_num).data(),
                  pattern_buffer.at(entry_num).size());

      search_entry.at(entry_num).cbAlign = allignment;
    }

    raw_context.vaMin = min_virtual_address;
    raw_context.ReadFlags = read_flags;
  };
};

std::expected<std::vector<uint8_t>, std::string> MemReadPage(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const uint64_t addr);

std::expected<std::vector<uint8_t>, std::string> MemReadEx(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const uint64_t addr, const std::size_t bytes,
    const uint32_t flag);

std::expected<bool, std::string> MemPrefetchPages(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const std::vector<uint64_t>& prefetch_addresses);

std::expected<void, std::string> MemWrite(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const uint64_t addr, const std::vector<uint8_t>& data);

std::expected<uint64_t, std::string> MemVirt2Phys(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const uint64_t addr);

std::expected<std::vector<uint64_t>, std::string> MemSearch(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const std::shared_ptr<MemorySearchContext>& context);

std::expected<uint32_t, std::string> MemReadScatter(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, PPMEM_SCATTER scatter_list, uint64_t scatter_size,
    uint32_t flags);

std::expected<uint32_t, std::string> MemWriteScatter(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, PPMEM_SCATTER scatter_list, uint64_t scatter_size);
};  // namespace VMM
