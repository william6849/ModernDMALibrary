#pragma once

namespace VMM {
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
  std::string process_name;
  std::string long_process_name;
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
    std::vector<int8_t> sid;
    VMMDLL_PROCESS_INTEGRITY_LEVEL integrity_level;
  } win;

  std::unordered_set<uint32_t> child_process_pid_list;
};

void HandleDeleter(VMM_HANDLE handle);

std::expected<uint32_t, std::string> PidGetFromName(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    std::string_view process_name);

std::expected<std::vector<uint32_t>, std::string> PidList(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle);

ProcessInformation ProcessGetInformation(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid);

std::expected<std::map<uint32_t, ProcessInformation>, std::string>
ProcessGetInformationAll(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle);

namespace PE {
std::expected<std::vector<_IMAGE_DATA_DIRECTORY>, std::string>
ProcessGetDirectories(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle, uint32_t pid,
    std::string_view module_name);

std::expected<std::vector<_IMAGE_SECTION_HEADER>, std::string>
ProcessGetSections(const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
                   uint32_t pid, std::string_view module_name);

}  // namespace PE

namespace MAP {
struct PteEntry {
  tdVMMDLL_MAP_PTEENTRY raw_entry;
  std::string name = "\0";
};
std::expected<std::vector<PteEntry>, std::string> GetPte(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const bool identify_modules);

struct VadEntry {
  tdVMMDLL_MAP_VADENTRY raw_entry;
  std::string name = "\0";
};
std::expected<std::vector<VadEntry>, std::string> GetVad(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const bool identify_modules);

struct VadExEntry {
  tdVMMDLL_MAP_VADEXENTRY raw_entry;
};
std::expected<std::vector<VadExEntry>, std::string> GetVadEx(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const uint32_t start_page, const uint32_t pages);

struct ModuleEntry {
  tdVMMDLL_MAP_MODULEENTRY raw_entry;
  std::string name = "\0";
  std::string full_name = "\0";
};
std::expected<std::vector<ModuleEntry>, std::string> GetModule(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const uint32_t flags);

std::expected<ModuleEntry, std::string> GetModule(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const uint32_t flags, const std::string& module_name);

struct UnloadModuleEntry {
  tdVMMDLL_MAP_UNLOADEDMODULEENTRY raw_entry;
  std::string name = "\0";
};
std::expected<std::vector<UnloadModuleEntry>, std::string> GetUnloadedModule(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle, uint32_t pid);

struct EatEntry {
  tdVMMDLL_MAP_EATENTRY raw_entry;
  std::string function = "\0";
  std::string forward_function = "\0";
};
std::expected<std::vector<EatEntry>, std::string> GetEAT(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle, uint32_t pid);

struct IatEntry {
  tdVMMDLL_MAP_IATENTRY raw_entry;
  std::string function = "\0";
  std::string module = "\0";
};
std::expected<std::vector<IatEntry>, std::string> GetIAT(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle, uint32_t pid);

struct HeapEntry {
  tdVMMDLL_MAP_HEAPENTRY raw_entry;
};
std::expected<std::vector<HeapEntry>, std::string> GetHeap(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid);

struct HeapAllocEntry {
  tdVMMDLL_MAP_HEAPALLOCENTRY raw_entry;
};
std::expected<std::vector<HeapAllocEntry>, std::string> GetHeapAlloc(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const uint64_t heap_number_or_address);

struct ThreadEntry {
  tdVMMDLL_MAP_THREADENTRY raw_entry;
};
std::expected<std::vector<ThreadEntry>, std::string> GetThread(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid);

struct HandleEntry {
  tdVMMDLL_MAP_HANDLEENTRY raw_entry;
  std::string name = "\0";
  std::string type = "\0";
};
std::expected<std::vector<HandleEntry>, std::string> GetHandle(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid);

};  // namespace MAP

};  // namespace VMM
