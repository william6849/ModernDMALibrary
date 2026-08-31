#include "process.h"

#include <ranges>

namespace VMM {
std::expected<uint32_t, std::string> PidGetFromName(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    std::string_view process_name) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  if (process_name.empty()) {
    return std::unexpected("Empty process name");
  }
  uint32_t pid = 0;

  bool ret =
      VMMDLL_PidGetFromName(handle->get(), std::string(process_name).c_str(),
                            static_cast<PDWORD>(&pid));
  if (!ret) {
    std::expected<uint32_t, std::string> Helper = [&]() {
      const auto process_map = ProcessGetInformationAll(handle);
      if (!process_map) {
        return std::unexpected(process_map.error());
      }
      for (const auto& [pid, info] : *process_map) {
        if (info.process_name == process_name) {
          return pid;
        }
      }
      return std::unexpected("No pid");
      ;
    };
    auto ret = Helper();
    if (!ret) {
      return std::unexpected("VMMDLL_PidGetFromName failed.");
    }
    return *ret;
  }
}

std::expected<std::vector<uint32_t>, std::string> PidList(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  SIZE_T pid_numbers = 0;

  bool ret = VMMDLL_PidList(handle->get(), nullptr, &pid_numbers);
  if (!ret || pid_numbers == 0) {
    return std::unexpected("Failed to get PID count or no PIDs available");
  }

  auto pid_list = std::array<DWORD, pid_numbers>;

  ret = VMMDLL_PidList(handle->get(), pid_list.data(), &pid_numbers);
  if (!ret) {
    return std::unexpected("VMMDLL_PidList failed");
  }

  auto return_list = pid_list | std::ranges::to<std::vector>();

  return return_list;
}

ProcessInformation ProcessGetInformation(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid) {
  auto ret = ProcessGetInformationAll(handle).at(static_cast<int32_t>(pid));
  if (!ret) {
    spdlog::error("No information {}", ret.error());
  }
  return *ret;
}

std::expected<std::map<uint32_t, ProcessInformation>, std::string>
ProcessGetInformationAll(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  DWORD information_counts = 0;
  PVMMDLL_PROCESS_INFORMATION information_list = nullptr;

  auto result = VMMDLL_ProcessGetInformationAll(
      handle->get(), &information_list, &information_counts);
  if (!result || information_list == nullptr || information_counts == 0) {
    return std::unexpected(
        "VMMDLL_ProcessGetInformationAll failed or returned no data");
  }

  auto list_memory_guard =
      std::unique_ptr<VMMDLL_PROCESS_INFORMATION, decltype(&VMMDLL_MemFree)>(
          information_list, VMMDLL_MemFree);

  std::map<uint32_t, ProcessInformation> process_information_map;

  for (int i = 0; i < information_counts; ++i) {
    const auto& entry = information_list[i];

    ProcessInformation info{
        .magic = entry.magic,
        .version = entry.wVersion,
        .size = entry.wSize,
        .memory_model = entry.tpMemoryModel,
        .system = entry.tpSystem,
        .user_only = static_cast<bool>(entry.fUserOnly),
        .pid = entry.dwPID,
        .parent_pid = entry.dwPPID,
        .state = entry.dwState,
        .directory_table_base = entry.paDTB,
        .directory_table_base_user_optional = entry.paDTB_UserOpt,
        .win =
            {
                .eprocess = entry.win.vaEPROCESS,
                .process_environment_block = entry.win.vaPEB,
                .reserved_1 = entry.win._Reserved1,
                .is_wow64 = static_cast<bool>(entry.win.fWow64),
                .process_environment_block_32 = entry.win.vaPEB32,
                .session_id = entry.win.dwSessionId,
                .luid = entry.win.qwLUID,
                .integrity_level = entry.win.IntegrityLevel,
            },
        .process_name = entry.szName,
        .long_process_name = entry.szNameLong,
    };

    info.win.sid.assign(entry.win.szSID,
                        entry.win.szSID + strlen(entry.win.szSID));

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

  return process_information_map;
}

namespace PE {
constexpr int32_t DEFAULT_PAGE_BYTES = 4096;
constexpr uint32_t IMAGE_DATA_DIRECTORY_SECTIONS = 16;

std::expected<std::vector<_IMAGE_DATA_DIRECTORY>, std::string>
ProcessGetDirectories(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle, uint32_t pid,
    std::string_view module_name) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  if (module_name.empty()) {
    return std::unexpected("Empty module name");
  }

  std::vector<_IMAGE_DATA_DIRECTORY> result(IMAGE_DATA_DIRECTORY_SECTIONS);

  if (!VMMDLL_ProcessGetDirectoriesU(handle->get(), pid,
                                     std::string(module_name).c_str(),
                                     result.data())) {
    return std::unexpected("VMMDLL_ProcessGetDirectoriesU failed");
  }

  return result;
}

std::expected<std::vector<_IMAGE_SECTION_HEADER>, std::string>
ProcessGetSections(const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
                   uint32_t pid, std::string_view module_name) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  if (module_name.empty()) {
    return std::unexpected("Empty module name");
  }

  DWORD section_count = 0;

  BOOL ok = VMMDLL_ProcessGetSectionsU(handle->get(), pid,
                                       std::string(module_name).c_str(),
                                       nullptr, 0, &section_count);

  if (!ok || section_count == 0) {
    return std::unexpected("Failed to query section count or invalid count");
  }

  std::vector<_IMAGE_SECTION_HEADER> result(section_count);

  ok = VMMDLL_ProcessGetSectionsU(handle->get(), pid,
                                  std::string(module_name).c_str(),
                                  result.data(), section_count, &section_count);

  if (!ok) {
    return std::unexpected("VMMDLL_ProcessGetSectionsU failed");
  }

  return result;
}

};  // namespace PE

namespace MAP {
using VmmMapPtePtr = std::unique_ptr<VMMDLL_MAP_PTE, VmmDeleter>;

std::expected<std::vector<PteEntry>, std::string> GetPte(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const bool identify_modules) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  PVMMDLL_MAP_PTE raw_pte = nullptr;

  if (!VMMDLL_Map_GetPteU(handle->get(), pid, identify_modules, &raw_pte)) {
    return std::unexpected("VMMDLL_Map_GetPteU call failed");
  }

  std::unique_ptr<tdVMMDLL_MAP_PTEENTRY, decltype(&VMMDLL_MemFree)> pte_maps(
      raw_pte, VMMDLL_MemFree);

  if (pte_maps->dwVersion != VMMDLL_MAP_PTE_VERSION) {
    return std::unexpected("GetPte version mismatched");
  }

  std::vector<PteEntry> result;
  result.reserve(pte_maps->cMap);

  for (DWORD i = 0; i < pte_maps->cMap; ++i) {
    const auto& pte = pte_maps->pMap[i];
    PteEntry entry;
    entry.raw_entry = pte;
    entry.name = pte.uszText ? std::string(pte.uszText) : "";
    entry.raw_entry.uszText = entry.name.c_str();
    result.emplace_back(std::move(entry));
  }

  return result;
}

std::expected<std::vector<VadEntry>, std::string> GetVad(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const bool identify_modules) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  PVMMDLL_MAP_VAD raw_vad = nullptr;

  if (!VMMDLL_Map_GetVadU(handle->get(), pid, identify_modules, &raw_vad)) {
    return std::unexpected("VMMDLL_Map_GetVadU failed");
  }

  std::unique_ptr<tdVMMDLL_MAP_VADENTRY, decltype(&VMMDLL_MemFree)> vad_maps(
      raw_vad, VMMDLL_MemFree);

  if (vad_maps->dwVersion != VMMDLL_MAP_VAD_VERSION) {
    return std::unexpected("VAD version mismatch");
  }

  std::vector<VadEntry> result;
  result.reserve(vad_maps->cMap);

  for (auto i = 0; i < vad_maps->cMap; ++i) {
    const auto& raw = vad_maps->pMap[i];

    VadEntry entry;
    entry.raw_entry = raw;
    entry.name = raw.uszText ? std::string(raw.uszText) : "";
    entry.raw_entry.uszText = entry.name.c_str();

    result.push_back(std::move(entry));
  }

  return result;
}

std::expected<std::vector<VadExEntry>, std::string> GetVadEx(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const uint32_t start_page, const uint32_t pages) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  PVMMDLL_MAP_VADEX raw_vadex = nullptr;

  if (!VMMDLL_Map_GetVadEx(handle->get(), pid, start_page, pages, &raw_vadex)) {
    return std::unexpected("VMMDLL_Map_GetVadEx failed");
  }

  std::unique_ptr<tdVMMDLL_MAP_VADEX, decltype(&VMMDLL_MemFree)> vadex_maps(
      raw_vadex, VMMDLL_MemFree);

  if (vadex_maps->dwVersion != VMMDLL_MAP_VADEX_VERSION) {
    return std::unexpected("GetVadEx version mismatched");
  }

  std::vector<VadExEntry> result;
  result.reserve(vadex_maps->cMap);

  for (auto i = 0; i < vadex_maps->cMap; ++i) {
    const auto& raw = vadex_maps->pMap[i];
    result.emplace_back(VadExEntry{.raw_entry = raw});
  }

  return result;
}

std::expected<std::vector<ModuleEntry>, std::string> GetModule(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const uint32_t flags) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  PVMMDLL_MAP_MODULE raw_module = nullptr;

  if (!VMMDLL_Map_GetModuleU(handle->get(), pid, &module_maps, flags)) {
    return std::unexpected("VMMDLL_Map_GetModuleU failed");
  }

  std::unique_ptr<tdVMMDLL_MAP_MODULEENTRY, decltype(&VMMDLL_MemFree)>
      module_maps(raw_module, VMMDLL_MemFree);

  if (module_maps->dwVersion != VMMDLL_MAP_MODULE_VERSION) {
    return std::unexpected("GetModule version mismatched");
  }

  std::vector<ModuleEntry> result;
  result.reserve(module_maps->cMap);

  for (auto i = 0; i < module_maps->cMap; ++i) {
    const auto& raw = module_maps->pMap[i];
    ModuleEntry entry = {
        .raw_entry = ret, .name = ret.uszText, .full_name = ret.uszFullName};
    entry.raw_entry.uszText = entry.name.c_str();
    entry.raw_entry.uszFullName = entry.full_name.c_str();
    result.emplace_back(std::move(entry));
  }

  return result;
}

std::expected<ModuleEntry, std::string> GetModule(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const uint32_t flags, const std::string& module_name) {
  auto map = GetModule(handle, pid, flags);
  if (!map) {
    std::unexpected(map.error());
  }
  for (auto& entry : *map) {
    if (entry.name == module_name || entry.full_name == module_name) {
      return entry;
    }
  }
  return std::unexpected("No sepecific module: " + module_name);
}

std::expected<std::vector<UnloadModuleEntry>, std::string> GetUnloadedModule(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle, uint32_t pid) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  PVMMDLL_MAP_UNLOADEDMODULE raw_ptr = nullptr;
  if (!VMMDLL_Map_GetUnloadedModuleU(handle->get(), pid, &raw_ptr)) {
    return std::unexpected("VMMDLL_Map_GetUnloadedModuleU failed");
  }

  std::unique_ptr<tdVMMDLL_MAP_UNLOADEDMODULEENTRY, decltype(&VMMDLL_MemFree)>
      unload_module_maps(raw_ptr, VMMDLL_MemFree);

  if (unload_module_maps->dwVersion != VMMDLL_MAP_UNLOADEDMODULE_VERSION) {
    return std::unexpected("GetUnloadedModule version mismatched");
  }

  std::vector<UnloadModuleEntry> result;
  result.reserve(unload_module_maps->cMap);

  for (size_t i = 0; i < unload_module_maps->cMap; ++i) {
    const auto& raw = unload_module_maps->pMap[i];
    UnloadModuleEntry entry;
    entry.raw_entry = raw;
    entry.name = raw.uszText ? raw.uszText : "";
    entry.raw_entry.uszText = entry.name.c_str();
    result.emplace_back(std::move(entry));
  }

  return result;
}

std::expected<std::vector<EatEntry>, std::string> GetEAT(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle, uint32_t pid) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  PVMMDLL_MAP_EAT raw_ptr = nullptr;
  if (!VMMDLL_Map_GetEATU(handle->get(), pid, &raw_ptr)) {
    return std::unexpected("VMMDLL_Map_GetEATU failed");
  }

  std::unique_ptr<tdVMMDLL_MAP_EATENTRY, decltype(&VMMDLL_MemFree)> eat_maps(
      raw_ptr, VMMDLL_MemFree);

  if (eat_maps->dwVersion != VMMDLL_MAP_EAT_VERSION) {
    return std::unexpected("GetEAT version mismatched");
  }

  std::vector<EatEntry> result;
  result.reserve(eat_maps->cMap);

  for (size_t i = 0; i < eat_maps->cMap; ++i) {
    const auto& raw = eat_maps->pMap[i];
    EatEntry entry;
    entry.raw_entry = raw;
    entry.function = ret.uszFunction;
    entry.forward_function = ret.uszForwardedFunction;

    entry.raw_entry.uszFunction = entry.function.c_str();
    entry.raw_entry.uszForwardedFunction = entry.forward_function.c_str();
    result.emplace_back(std::move(entry));
  }

  return result;
}

std::expected<std::vector<IatEntry>, std::string> GetIAT(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle, uint32_t pid) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  PVMMDLL_MAP_IAT raw_ptr = nullptr;
  if (!VMMDLL_Map_GetIATU(handle->get(), pid, &raw_ptr)) {
    return std::unexpected("VMMDLL_Map_GetIATU failed");
  }

  std::unique_ptr<tdVMMDLL_MAP_IATENTRY, decltype(&VMMDLL_MemFree)> iat_maps(
      raw_ptr, VMMDLL_MemFree);

  if (iat_maps->dwVersion != VMMDLL_MAP_IAT_VERSION) {
    return std::unexpected("GetIAT version mismatched");
  }

  std::vector<IatEntry> result;
  result.reserve(iat_maps->cMap);

  for (size_t i = 0; i < iat_maps->cMap; ++i) {
    const auto& raw = iat_maps->pMap[i];
    EatEntry entry;
    entry.raw_entry = raw;
    entry.function = ret.uszFunction;
    entry.module = ret.uszModule;

    entry.raw_entry.uszFunction = entry.function.c_str();
    entry.raw_entry.uszModule = entry.module.c_str();
    result.emplace_back(std::move(entry));
  }

  return result;
}

std::expected<std::vector<HeapEntry>, std::string> GetHeap(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  PVMMDLL_MAP_HEAP raw_ptr = nullptr;
  if (!VMMDLL_Map_GetHeap(handle->get(), pid, &raw_ptr)) {
    return std::unexpected("VMMDLL_Map_GetHeap failed");
  }

  std::unique_ptr<tdVMMDLL_MAP_HEAPENTRY, decltype(&VMMDLL_MemFree)> heap_maps(
      raw_ptr, VMMDLL_MemFree);

  if (heap_maps->dwVersion != VMMDLL_MAP_HEAP_VERSION) {
    return std::unexpected("GetHeap version mismatched");
  }

  std::vector<HeapEntry> result;
  result.reserve(heap_maps->cMap);

  for (size_t i = 0; i < heap_maps->cMap; i++) {
    const auto& raw = heap_maps->pMap[i];
    HeapEntry entry = {.raw_entry = ret};
    result.emplace_back(std::move(entry));
  }

  return result;
}

std::expected<std::vector<HeapAllocEntry>, std::string> GetHeapAlloc(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid, const uint64_t heap_number_or_address) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  PVMMDLL_MAP_HEAPALLOC raw_ptr = nullptr;
  if (!VMMDLL_Map_GetHeapAlloc(handle->get(), pid, heap_number_or_address,
                               &raw_ptr)) {
    return std::unexpected("VMMDLL_Map_GetHeapAlloc failed");
  }

  std::unique_ptr<tdVMMDLL_MAP_HEAPALLOCENTRY, decltype(&VMMDLL_MemFree)>
      heapalloc_maps(raw_ptr, VMMDLL_MemFree);

  if (heapalloc_maps->dwVersion != VMMDLL_MAP_HEAPALLOC_VERSION) {
    return std::unexpected("GetHeapAlloc version mismatched");
  }

  std::vector<HeapEntry> result;
  result.reserve(heap_maps->cMap);

  for (size_t i = 0; i < heapalloc_maps->cMap; i++) {
    const auto& raw = heapalloc_maps->pMap[i];
    HeapEntry entry = {.raw_entry = ret};
    result.emplace_back(std::move(entry));
  }

  return result;
}

std::expected<std::vector<ThreadEntry>, std::string> GetThread(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  PVMMDLL_MAP_THREAD raw_ptr = nullptr;
  if (!VMMDLL_Map_GetThread(handle->get(), pid, &raw_ptr)) {
    return std::unexpected("VMMDLL_Map_GetThread failed");
  }

  std::unique_ptr<tdVMMDLL_MAP_THREADENTRY, decltype(&VMMDLL_MemFree)>
      thread_maps(raw_ptr, VMMDLL_MemFree);

  if (thread_maps->dwVersion != VMMDLL_MAP_THREAD_VERSION) {
    return std::unexpected("GetThread version mismatched");
  }

  std::vector<ThreadEntry> result;
  result.reserve(thread_maps->cMap);

  for (size_t i = 0; i < thread_maps->cMap; i++) {
    const auto& raw = thread_maps->pMap[i];
    ThreadEntry entry = {.raw_entry = ret};
    result.emplace_back(std::move(entry));
  }

  return result;
}

std::expected<std::vector<HandleEntry>, std::string> GetHandle(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle,
    const uint32_t pid) {
  if (!handle || !handle->get()) {
    return std::unexpected("Invalid handle");
  }

  PVMMDLL_MAP_HANDLE raw_ptr = nullptr;
  if (!VMMDLL_Map_GetHandleU(handle->get(), pid, &raw_ptr)) {
    return std::unexpected("VMMDLL_Map_GetHandleU failed");
  }

  std::unique_ptr<tdVMMDLL_MAP_HANDLEENTRY, decltype(&VMMDLL_MemFree)>
      handle_maps(raw_ptr, VMMDLL_MemFree);

  if (handle_maps->dwVersion != VMMDLL_MAP_HANDLE_VERSION) {
    return std::unexpected("GetHandle version mismatched");
  }

  std::vector<HandleEntry> result;
  result.reserve(handle_maps->cMap);

  for (size_t i = 0; i < handle_maps->cMap; i++) {
    const auto& raw = handle_maps->pMap[i];
    HandleEntry entry = {
        .raw_entry = ret, .name = ret.uszText, .type = ret.uszType};
    entry.raw_entry.uszText = &entry.name.at(0);
    entry.raw_entry.uszType = &entry.type.at(0);
    result.emplace_back(std::move(entry));
  }

  return result;
}

};  // namespace MAP
};  // namespace VMM
