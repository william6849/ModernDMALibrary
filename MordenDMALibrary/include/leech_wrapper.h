#ifndef LEECH_WRAPPER_H
#define LEECH_WRAPPER_H

#include <array>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <unordered_set>
#include <vector>

#include "vmmdll.h"

const int32_t DEFAULT_PAGE_BYTES = 4096;

template <typename T>
class HandleWrapper {
 public:
  HandleWrapper(T* handle) : handle_(handle, nullptr) {}
  HandleWrapper(T* handle, std::function<void(T*)> deleter)
      : handle_(handle, deleter) {}

  void reset(T* handle) { handle_.reset(handle); }

  T* get() const { return handle_.get(); }

  operator T*() const { return handle_.get(); }

 private:
  std::unique_ptr<T, std::function<void(T*)>> handle_;
};

namespace LC {
void HandleDeleter(HANDLE handle);
};
namespace VMM {

void HandleDeleter(VMM_HANDLE handle);

VMM_HANDLE Initialize(const std::string& arguments);

bool ConfigGet(const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
               uint64_t, uint64_t&);
bool ConfigSet(const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
               uint64_t, uint64_t);

std::optional<std::vector<uint8_t>> MemReadPage(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const uint64_t addr);

std::optional<std::vector<uint8_t>> MemReadPage(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const uint64_t addr);

bool MemPrefetchPages(const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
                      const uint32_t pid,
                      std::vector<uint64_t> prefetch_addresses);

std::optional<uint64_t> MemVirt2Phys(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const uint64_t addr);

std::optional<std::vector<uint8_t>> MemReadEx(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const uint64_t addr, const std::size_t bytes,
    const uint32_t flag);

bool MemWrite(const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
              const uint32_t pid, const uint64_t addr,
              std::vector<uint8_t>& data);

uint32_t MemReadScatter(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, PPMEM_SCATTER scatter_list, int32_t scatter_size,
    int32_t flags);
uint32_t MemWriteScatter(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, PPMEM_SCATTER scatter_list, int32_t scatter_size);

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
      throw std::runtime_error("Empty bytes");
    }
    if (targets.at(0).size() > VMMDLL_MEM_SEARCH_MAXLENGTH) {
      throw std::runtime_error("Search length exceed");
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

std::vector<uint64_t> MemSearch(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const std::shared_ptr<MemorySearchContext> context);

struct ProcessInformation {
  uint64_t magic;
  uint16_t version;
  uint16_t size;
  VMMDLL_MEMORYMODEL_TP memory_model;
  VMMDLL_SYSTEM_TP system;
  bool user_only;
  uint32_t pid;
  uint32_t parent_pid;
  uint32_t state;
  std::array<int8_t, 16> process_name;
  std::array<int8_t, 64> long_process_name;
  uint64_t directory_table_base;
  uint64_t directory_table_base_user_optional;
  struct {
    uint64_t eprocess;
    uint64_t process_environment_block;
    uint64_t reserved_1;
    bool is_wow64;
    uint32_t process_environment_block_32;
    uint32_t session_id;
    uint64_t luid;
    std::array<int8_t, MAX_PATH> sid;
    VMMDLL_PROCESS_INTEGRITY_LEVEL integrity_level;
  } win;

  std::unordered_set<uint32_t> child_process_pid_list;
};

auto ProcessGetInformationAll(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle);

uint32_t PidGetFromName(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    std::string process_name);

std::vector<uint32_t> PidList(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle);

ProcessInformation ProcessGetInformation(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid);

namespace PE {
std::vector<_IMAGE_DATA_DIRECTORY> ProcessGetDirectories(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const std::string& module_name);

std::vector<_IMAGE_SECTION_HEADER> ProcessGetSections(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const std::string& module_name);
}  // namespace PE

namespace MAP {
struct PteEntry {
  tdVMMDLL_MAP_PTEENTRY raw_entry;
  std::string name = "\0";
};
std::vector<PteEntry> GetPte(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const bool identify_modules);

struct VadEntry {
  tdVMMDLL_MAP_VADENTRY raw_entry;
  std::string name = "\0";
};
std::vector<VadEntry> GetVad(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const bool identify_modules);

struct ModuleEntry {
  tdVMMDLL_MAP_MODULEENTRY raw_entry;
  std::string name = "\0";
  std::string full_name = "\0";
};
std::vector<ModuleEntry> GetModule(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, uint32_t flags);

struct UnloadModuleEntry {
  tdVMMDLL_MAP_UNLOADEDMODULEENTRY raw_entry;
  std::string name = "\0";
};
std::vector<UnloadModuleEntry> GetUnloadedModule(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid);

struct EatEntry {
  tdVMMDLL_MAP_EATENTRY raw_entry;
  std::string function = "\0";
  std::string forward_function = "\0";
};
std::vector<EatEntry> GetEAT(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, std::string module_name);

struct IatEntry {
  tdVMMDLL_MAP_IATENTRY raw_entry;
  std::string function = "\0";
  std::string module = "\0";
};
std::vector<IatEntry> GetIAT(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, std::string module_name);

}  // namespace MAP

};  // namespace VMM

#endif
