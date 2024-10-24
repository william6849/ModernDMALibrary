#include "leech_wrapper.h"

#include <optional>
#include <sstream>
#include <stdexcept>

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

VMM_HANDLE Initialize(const std::string& params) {
  spdlog::debug("Parameters: {}", params);

  std::stringstream params_stream(params);
  std::vector<std::string> params_parsed;
  std::string token;
  while (getline(params_stream, token, ' ')) {
    params_parsed.push_back(token);
  }

  std::vector<LPCSTR> c_str_vec;
  for (const auto& param : params_parsed) {
    c_str_vec.push_back(param.c_str());
  }

  return VMMDLL_Initialize(c_str_vec.size(), c_str_vec.data());
}

std::optional<std::vector<uint8_t>> MemReadEx(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const uint64_t addr, const std::size_t bytes,
    const uint32_t flag) {
  std::vector<uint8_t> ret(bytes);
  DWORD read_bytes = 0;
  auto result = VMMDLL_MemReadEx(
      handle->get(), static_cast<DWORD>(pid), static_cast<ULONG64>(addr),
      static_cast<PBYTE>(ret.data()), static_cast<DWORD>(bytes), &read_bytes,
      static_cast<DWORD>(flag));
  if (!result) {
    return {};
  }
  return std::move(ret);
}

bool MemWrite(const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
              const uint32_t pid, const uint64_t addr,
              std::vector<uint8_t>& data) {
  bool result = VMMDLL_MemWrite(
      handle->get(), static_cast<DWORD>(pid), static_cast<ULONG64>(addr),
      static_cast<PBYTE>(data.data()), static_cast<DWORD>(data.size()));
  return result;
}

bool ConfigGet(const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
               uint64_t opt, uint64_t& val) {
  ULONG64 val_t = 0;
  auto ret = VMMDLL_ConfigGet(handle->get(), opt, &val_t);
  val = val_t;
  return ret;
}

bool ConfigSet(const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
               uint64_t opt, uint64_t val) {
  return VMMDLL_ConfigSet(handle->get(), opt, val);
}

std::optional<std::vector<uint8_t>> MemReadPage(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const uint64_t addr) {
  std::vector<uint8_t> out_buffer(DEFAULT_PAGE_BYTES);
  auto result = VMMDLL_MemReadPage(handle->get(), static_cast<DWORD>(pid),
                                   static_cast<ULONG64>(addr),
                                   static_cast<PBYTE>(out_buffer.data()));
  if (!result) {
    return {};
  }
  return std::move(out_buffer);
}

bool MemPrefetchPages(const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
                      const uint32_t pid,
                      std::vector<uint64_t> prefetch_addresses) {
  std::vector<unsigned long long> vec_ull(prefetch_addresses.begin(),
                                          prefetch_addresses.end());
  return VMMDLL_MemPrefetchPages(handle->get(), static_cast<DWORD>(pid),
                                 vec_ull.data(), vec_ull.size());
}

std::optional<uint64_t> MemVirt2Phys(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const uint64_t addr) {
  ULONG64 result = 0;
  auto ret = VMMDLL_MemVirt2Phys(handle->get(), static_cast<DWORD>(pid),
                                 static_cast<ULONG64>(addr), &result);
  if (!ret) {
    return {};
  }
  return result;
}

std::vector<uint64_t> MemSearch(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const std::shared_ptr<MemorySearchContext> context) {
  DWORD search_result_count = 0;
  PQWORD search_result_ptr = nullptr;
  auto ret = VMMDLL_MemSearch(handle->get(), static_cast<DWORD>(pid),
                              &context->raw_context, &search_result_ptr,
                              &search_result_count);

  std::vector<uint64_t> result;
  if (ret) {
    for (auto entry = 0; entry < search_result_count; entry++) {
      result.push_back(search_result_ptr[entry]);
    }
    VMMDLL_MemFree(search_result_ptr);
  }
  return result;
}

uint32_t MemReadScatter(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, PPMEM_SCATTER scatter_list, uint64_t scatter_size,
    uint32_t flags) {
  return VMMDLL_MemReadScatter(handle->get(), pid, scatter_list,
                               static_cast<DWORD>(scatter_size), flags);
}

uint32_t MemWriteScatter(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, PPMEM_SCATTER scatter_list, uint64_t scatter_size) {
  return VMMDLL_MemWriteScatter(handle->get(), pid, scatter_list,
                                static_cast<DWORD>(scatter_size));
}

auto ProcessGetInformationAll(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle) {
  DWORD information_counts = 0;
  PVMMDLL_PROCESS_INFORMATION information_list = nullptr;
  auto result = VMMDLL_ProcessGetInformationAll(
      handle->get(), &information_list, &information_counts);
  std::map<int32_t, ProcessInformation> process_information_map;
  if (result) {
    for (int working_item = 0; working_item < information_counts;
         working_item++) {
      auto entry = &information_list[working_item];

      ProcessInformation information{
          .magic = entry->magic,
          .version = entry->wVersion,
          .size = entry->wSize,
          .memory_model = entry->tpMemoryModel,
          .system = entry->tpSystem,
          .user_only = static_cast<bool>(entry->fUserOnly),
          .pid = entry->dwPID,
          .parent_pid = entry->dwPPID,
          .state = entry->dwState,
          .directory_table_base = entry->paDTB,
          .directory_table_base_user_optional = entry->paDTB_UserOpt,
          .win = {.eprocess = entry->win.vaEPROCESS,
                  .process_environment_block = entry->win.vaPEB,
                  .reserved_1 = entry->win._Reserved1,
                  .is_wow64 = static_cast<bool>(entry->win.fWow64),
                  .process_environment_block_32 = entry->win.vaPEB32,
                  .session_id = entry->win.dwSessionId,
                  .luid = entry->win.qwLUID,
                  .integrity_level = entry->win.IntegrityLevel}};

      information.process_name = entry->szName;
      information.long_process_name = entry->szNameLong;
      std::string sid_str = entry->win.szSID;
      information.win.sid.assign(sid_str.begin(), sid_str.end());

      if (process_information_map.contains(entry->dwPID)) {
        information.child_process_pid_list = std::move(
            process_information_map.at(static_cast<int32_t>(entry->dwPID))
                .child_process_pid_list);
      }

      process_information_map.at(static_cast<int32_t>(entry->dwPID)) =
          std::move(information);

      process_information_map.at(entry->dwPPID)
          .child_process_pid_list.insert(entry->dwPID);
    }
    VMMDLL_MemFree(information_list);
  }
  return process_information_map;
}

uint32_t PidGetFromName(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    std::string process_name) {
  auto list = ProcessGetInformationAll(handle);
  for (const auto& entry : list) {
    if (std::equal(entry.second.process_name.begin(),
                   entry.second.process_name.end(), process_name.begin())) {
      return entry.first;
    }
  }
  return 0;
}

std::vector<uint32_t> PidList(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle) {
  std::vector<DWORD> pid_list;
  SIZE_T pid_numbers = 0;
  if (VMMDLL_PidList(handle->get(), nullptr, &pid_numbers)) {
    pid_list.resize(pid_numbers);
    if (VMMDLL_PidList(handle->get(), pid_list.data(), &pid_numbers)) {
    }
  }
  std::vector<uint32_t> return_list(pid_list.begin(), pid_list.end());
  return std::move(return_list);
}

ProcessInformation ProcessGetInformation(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid) {
  return ProcessGetInformationAll(handle).at(static_cast<int32_t>(pid));
}
namespace PE {
std::vector<_IMAGE_DATA_DIRECTORY> ProcessGetDirectories(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const std::string& module_name) {
  std::vector<_IMAGE_DATA_DIRECTORY> result(IMAGE_DATA_DIRECTORY_SECTIONS);
  if (VMMDLL_ProcessGetDirectoriesU(handle->get(), pid, module_name.c_str(),
                                    result.data())) {
    return result;
  }
  throw std::runtime_error("ProcessGetInformation");
}

std::vector<_IMAGE_SECTION_HEADER> ProcessGetSections(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const std::string& module_name) {
  std::vector<_IMAGE_SECTION_HEADER> result;
  DWORD sections = 0;
  if (VMMDLL_ProcessGetSectionsU(handle->get(), pid, module_name.c_str(),
                                 nullptr, 0, &sections)) {
    result.resize(sections);
    if (!VMMDLL_ProcessGetSectionsU(handle->get(), pid, module_name.c_str(),
                                    result.data(), sections, &sections)) {
      throw std::runtime_error("ProcessGetSectionsU");
    }
  }
  return result;
}
}  // namespace PE

namespace MAP {
std::vector<PteEntry> GetPte(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const bool identify_modules) {
  PVMMDLL_MAP_PTE pte_maps = nullptr;
  std::vector<PteEntry> result;
  auto ret =
      VMMDLL_Map_GetPteU(handle->get(), pid, identify_modules, &pte_maps);
  if (ret) {
    if (pte_maps->dwVersion != VMMDLL_MAP_PTE_VERSION) {
      VMMDLL_MemFree(pte_maps);
      throw std::runtime_error("GetPte version mismatched");
    }
    result.resize(pte_maps->cMap);
    for (auto entry_count = 0; entry_count < pte_maps->cMap; entry_count++) {
      auto ret = pte_maps->pMap[entry_count];
      PteEntry entry = {.raw_entry = ret, .name = ret.uszText};
      entry.raw_entry.uszText = &entry.name.at(0);
      result.push_back(entry);
    }
    VMMDLL_MemFree(pte_maps);
  }
  return result;
}

std::vector<VadEntry> GetVad(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const bool identify_modules) {
  PVMMDLL_MAP_VAD vad_maps = nullptr;
  std::vector<VadEntry> result;
  auto ret =
      VMMDLL_Map_GetVadU(handle->get(), pid, identify_modules, &vad_maps);
  if (ret) {
    if (vad_maps->dwVersion != VMMDLL_MAP_VAD_VERSION) {
      VMMDLL_MemFree(vad_maps);
      throw std::runtime_error("GetVad version mismatched");
    }
    result.resize(vad_maps->cMap);
    for (auto entry_count = 0; entry_count < vad_maps->cMap; entry_count++) {
      auto ret = vad_maps->pMap[entry_count];
      VadEntry entry = {.raw_entry = ret, .name = ret.uszText};
      entry.raw_entry.uszText = &entry.name.at(0);
      result.push_back(entry);
    }
    VMMDLL_MemFree(vad_maps);
  }
  return result;
}

std::vector<VadExEntry> GetVadEx(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const uint32_t start_page, const uint32_t pages) {
  PVMMDLL_MAP_VADEX vadex_maps = nullptr;
  std::vector<VadExEntry> result;
  auto ret =
      VMMDLL_Map_GetVadEx(handle->get(), pid, start_page, pages, &vadex_maps);
  if (ret) {
    if (vadex_maps->dwVersion != VMMDLL_MAP_VADEX_VERSION) {
      VMMDLL_MemFree(vadex_maps);
      throw std::runtime_error("GetVadEx version mismatched");
    }
    result.resize(vadex_maps->cMap);
    for (auto entry_count = 0; entry_count < vadex_maps->cMap; entry_count++) {
      auto ret = vadex_maps->pMap[entry_count];
      VadExEntry entry = {.raw_entry = ret};
      result.push_back(entry);
    }
    VMMDLL_MemFree(vadex_maps);
  }
  return result;
}

std::vector<ModuleEntry> GetModule(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const uint32_t flags) {
  PVMMDLL_MAP_MODULE module_maps = nullptr;
  std::vector<ModuleEntry> result;
  auto ret = VMMDLL_Map_GetModuleU(handle->get(), pid, &module_maps, flags);
  if (ret) {
    if (module_maps->dwVersion != VMMDLL_MAP_MODULE_VERSION) {
      VMMDLL_MemFree(module_maps);
      throw std::runtime_error("GetModule version mismatched");
    }
    result.resize(module_maps->cMap);
    for (auto entry_count = 0; entry_count < module_maps->cMap; entry_count++) {
      auto ret = module_maps->pMap[entry_count];
      ModuleEntry entry = {
          .raw_entry = ret, .name = ret.uszText, .full_name = ret.uszFullName};
      entry.raw_entry.uszText = &entry.name.at(0);
      entry.raw_entry.uszFullName = &entry.full_name.at(0);
      result.push_back(entry);
    }
    VMMDLL_MemFree(module_maps);
  }
  return result;
}

std::optional<ModuleEntry> GetModule(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const uint32_t flags, const std::string& module_name) {
  auto map = GetModule(handle, pid, flags);
  for (auto& entry : map) {
    if (entry.name == module_name || entry.full_name == module_name) {
      return entry;
    }
  }
  return {};
}

std::vector<UnloadModuleEntry> GetUnloadedModule(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid) {
  PVMMDLL_MAP_UNLOADEDMODULE unload_module_maps = nullptr;
  std::vector<UnloadModuleEntry> result;
  auto ret =
      VMMDLL_Map_GetUnloadedModuleU(handle->get(), pid, &unload_module_maps);
  if (ret) {
    if (unload_module_maps->dwVersion != VMMDLL_MAP_UNLOADEDMODULE_VERSION) {
      VMMDLL_MemFree(unload_module_maps);
      throw std::runtime_error("GetUnloadedModule version mismatched");
    }
    result.resize(unload_module_maps->cMap);
    for (auto entry_count = 0; entry_count < unload_module_maps->cMap;
         entry_count++) {
      auto ret = unload_module_maps->pMap[entry_count];
      UnloadModuleEntry entry = {.raw_entry = ret, .name = ret.uszText};
      entry.raw_entry.uszText = &entry.name.at(0);
      result.push_back(entry);
    }
    VMMDLL_MemFree(unload_module_maps);
  }
  return result;
}

std::vector<EatEntry> GetEAT(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, std::string module_name) {
  PVMMDLL_MAP_EAT eat_maps = nullptr;
  std::vector<EatEntry> result;
  auto ret =
      VMMDLL_Map_GetEATU(handle->get(), pid, module_name.data(), &eat_maps);
  if (ret) {
    if (eat_maps->dwVersion != VMMDLL_MAP_EAT_VERSION) {
      VMMDLL_MemFree(eat_maps);
      throw std::runtime_error("GetEat version mismatched");
    }
    result.resize(eat_maps->cMap);
    for (auto entry_count = 0; entry_count < eat_maps->cMap; entry_count++) {
      auto ret = eat_maps->pMap[entry_count];
      EatEntry entry = {.raw_entry = ret,
                        .function = ret.uszFunction,
                        .forward_function = ret.uszForwardedFunction};
      entry.raw_entry.uszFunction = &entry.function.at(0);
      entry.raw_entry.uszForwardedFunction = &entry.forward_function.at(0);
      result.push_back(entry);
    }
    VMMDLL_MemFree(eat_maps);
  }
  return result;
}

std::vector<IatEntry> GetIAT(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, std::string module_name) {
  PVMMDLL_MAP_IAT iat_maps = nullptr;
  std::vector<IatEntry> result;
  auto ret =
      VMMDLL_Map_GetIATU(handle->get(), pid, module_name.data(), &iat_maps);
  if (ret) {
    if (iat_maps->dwVersion != VMMDLL_MAP_IAT_VERSION) {
      VMMDLL_MemFree(iat_maps);
      throw std::runtime_error("GetIat version mismatched");
    }
    result.resize(iat_maps->cMap);
    for (auto entry_count = 0; entry_count < iat_maps->cMap; entry_count++) {
      auto ret = iat_maps->pMap[entry_count];
      IatEntry entry = {.raw_entry = ret,
                        .function = ret.uszFunction,
                        .module = ret.uszModule};
      entry.raw_entry.uszFunction = &entry.function.at(0);
      entry.raw_entry.uszModule = &entry.module.at(0);
      result.push_back(entry);
    }
    VMMDLL_MemFree(iat_maps);
  }
  return result;
}

std::vector<HeapEntry> GetHeap(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid) {
  PVMMDLL_MAP_HEAP heap_maps = nullptr;
  std::vector<HeapEntry> result;
  auto ret = VMMDLL_Map_GetHeap(handle->get(), pid, &heap_maps);
  if (ret) {
    if (heap_maps->dwVersion != VMMDLL_MAP_HEAP_VERSION) {
      VMMDLL_MemFree(heap_maps);
      throw std::runtime_error("GetHeap version mismatched");
    }
    result.resize(heap_maps->cMap);
    for (auto entry_count = 0; entry_count < heap_maps->cMap; entry_count++) {
      auto ret = heap_maps->pMap[entry_count];
      HeapEntry entry = {.raw_entry = ret};
      result.push_back(entry);
    }
    VMMDLL_MemFree(heap_maps);
  }
  return result;
}

std::vector<HeapAllocEntry> GetHeapAlloc(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const uint64_t heap_number_or_address) {
  PVMMDLL_MAP_HEAPALLOC heap_alloc_maps = nullptr;
  std::vector<HeapAllocEntry> result;
  auto ret = VMMDLL_Map_GetHeapAlloc(handle->get(), pid, heap_number_or_address,
                                     &heap_alloc_maps);
  if (ret) {
    if (heap_alloc_maps->dwVersion != VMMDLL_MAP_HEAPALLOC_VERSION) {
      VMMDLL_MemFree(heap_alloc_maps);
      throw std::runtime_error("GetHeapAlloc version dismatch");
    }
    result.resize(heap_alloc_maps->cMap);
    for (auto entry_count = 0; entry_count < heap_alloc_maps->cMap;
         entry_count++) {
      auto ret = heap_alloc_maps->pMap[entry_count];
      HeapAllocEntry entry = {.raw_entry = ret};
      result.push_back(entry);
    }
    VMMDLL_MemFree(heap_alloc_maps);
  }
  return result;
}

std::vector<ThreadEntry> GetThread(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid) {
  PVMMDLL_MAP_THREAD thread_maps = nullptr;
  std::vector<ThreadEntry> result;
  auto ret = VMMDLL_Map_GetThread(handle->get(), pid, &thread_maps);
  if (ret) {
    if (thread_maps->dwVersion != VMMDLL_MAP_THREAD_VERSION) {
      VMMDLL_MemFree(thread_maps);
      throw std::runtime_error("GetThread version mismatched");
    }
    result.resize(thread_maps->cMap);
    for (auto entry_count = 0; entry_count < thread_maps->cMap; entry_count++) {
      auto ret = thread_maps->pMap[entry_count];
      ThreadEntry entry = {.raw_entry = ret};
      result.push_back(entry);
    }
    VMMDLL_MemFree(thread_maps);
  }
  return result;
}

std::vector<HandleEntry> GetHandle(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid) {
  PVMMDLL_MAP_HANDLE handle_maps = nullptr;
  std::vector<HandleEntry> result;
  auto ret = VMMDLL_Map_GetHandleU(handle->get(), pid, &handle_maps);
  if (ret) {
    if (handle_maps->dwVersion != VMMDLL_MAP_HANDLE_VERSION) {
      VMMDLL_MemFree(handle_maps);
      throw std::runtime_error("GetHandle version mismatched");
    }
    result.resize(handle_maps->cMap);
    for (auto entry_count = 0; entry_count < handle_maps->cMap; entry_count++) {
      auto ret = handle_maps->pMap[entry_count];
      HandleEntry entry = {
          .raw_entry = ret, .name = ret.uszText, .type = ret.uszType};
      entry.raw_entry.uszText = &entry.name.at(0);
      entry.raw_entry.uszType = &entry.type.at(0);
      result.push_back(entry);
    }
    VMMDLL_MemFree(handle_maps);
  }
  return result;
}

}  // namespace MAP

}  // namespace VMM
