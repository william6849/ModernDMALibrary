#include "leech_wrapper.h"

#include <optional>
#include <sstream>

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

std::optional<std::vector<std::vector<uint64_t>>> MemSearch(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const std::shared_ptr<MemorySearchContext> context) {
  DWORD search_result_count = 0;
  PQWORD search_result_ptr = nullptr;
  auto ret = VMMDLL_MemSearch(handle->get(), static_cast<DWORD>(pid),
                              &context->raw_context, &search_result_ptr,
                              &search_result_count);
  if (!ret) {
    return {};
  }
  std::vector<std::vector<uint64_t>> result;
  return result;
}

uint32_t MemReadScatter(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, PPMEM_SCATTER scatter_list, int32_t scatter_size,
    int32_t flags) {
  return VMMDLL_MemReadScatter(handle->get(), pid, scatter_list, scatter_size,
                               flags);
}

uint32_t MemWriteScatter(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, PPMEM_SCATTER scatter_list, int32_t scatter_size) {
  return VMMDLL_MemWriteScatter(handle->get(), pid, scatter_list, scatter_size);
}

Scatter::Scatter(uint32_t pid, uint32_t flags = VMMDLL_FLAG_NOCACHE |
                                                VMMDLL_FLAG_ZEROPAD_ON_FAIL |
                                                VMMDLL_FLAG_NOPAGING)
    : pid_(pid), flags_(flags) {}

void Scatter::AddSRP(const SRP& srp) { srp_map_[srp.address] = srp; }

void Scatter::AddSRP(const std::vector<SRP>& srps) {
  for (auto& srp : srps) {
    AddSRP(srp);
  }
}

void Scatter::AddSRP(int64_t address, int32_t length) {
  SRP srp;
  srp.address = address;
  srp.length = length;
  srp.scatter.qwA = address;
  srp.scatter.cb = length;
  srp.scatter.pb = srp.buffer.data();
  AddSRP(srp);
}

bool Scatter::RemoveSRP(const SRP& srp) { return RemoveSRP(srp.address); }

bool Scatter::RemoveSRP(int64_t address) {
  auto it = srp_map_.find(address);
  if (it == srp_map_.end()) {
    return false;
  }
  srp_map_.erase(it);
  return true;
}

SRP* Scatter::GetSRP(int64_t address) {
  auto it = srp_map_.find(address);
  if (it == srp_map_.end()) {
    return nullptr;
  }
  return &(it->second);
}

std::vector<uint8_t>& Scatter::GetData(int64_t address) {
  auto it = srp_map_.find(address);
  if (it == srp_map_.end()) {
    throw std::runtime_error("SRP not exist.");
  }
  return it->second.buffer;
}

const std::map<int64_t, SRP>& Scatter::SRPMap() noexcept { return srp_map_; }

bool Scatter::ExecuteRead() { return false; }

bool Scatter::ExecuteWrite() { return false; }

void Scatter::SetPID(uint32_t pid) { pid_ = pid; }
void Scatter::SetFlags(uint32_t flags) { flags_ = flags; }

auto ProcessGetInformationAll(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle) {
  DWORD information_counts = 0;
  PVMMDLL_PROCESS_INFORMATION information_list = nullptr;
  printf("CALL:    VMMDLL_ProcessGetInformationAll\n");
  auto result = VMMDLL_ProcessGetInformationAll(
      handle->get(), &information_list, &information_counts);
  std::map<int32_t, PROCESS_INFORMATION> process_information_map;
  if (result) {
    for (int working_item = 0; working_item < information_counts;
         working_item++) {
      auto entry = &information_list[working_item];

      PROCESS_INFORMATION information{
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

      std::copy(entry->szName,
                entry->szName + std::min(information.process_name.size() - 1,
                                         strlen(entry->szName)),
                information.process_name.begin());
      information.process_name[std::min(information.process_name.size() - 1,
                                        strlen(entry->szName))] = '\0';
      std::copy(
          entry->szNameLong,
          entry->szNameLong + std::min(information.long_process_name.size() - 1,
                                       strlen(entry->szNameLong)),
          information.long_process_name.begin());
      information
          .long_process_name[std::min(information.long_process_name.size() - 1,
                                      strlen(entry->szNameLong))] = '\0';
      std::copy(entry->win.szSID,
                entry->win.szSID + std::min(information.win.sid.size() - 1,
                                            strlen(entry->win.szSID)),
                information.win.sid.begin());
      information.win.sid[std::min(information.win.sid.size() - 1,
                                   strlen(entry->win.szSID))] = '\0';

      if (process_information_map.contains(entry->dwPID)) {
        information.child_process_pid_list = std::move(
            process_information_map.at(entry->dwPID).child_process_pid_list);
      }

      process_information_map[entry->dwPID] = std::move(information);

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
  for (auto& entry : list) {
    if (std::equal(entry.second.process_name.begin(),
                   entry.second.process_name.begin() + process_name.size(),
                   process_name.begin())) {
      return entry.first;
    }
  }
  return 0;
}

std::vector<uint32_t> PidList(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle) {
  std::vector<uint32_t> ret_pid_list;
  PDWORD pid_list;
  SIZE_T pid_number;
  if (VMMDLL_PidList(handle->get(), pid_list, &pid_number)) {
    for (auto count = 0; count < pid_number; count++) {
      ret_pid_list.push_back(pid_list[count]);
    }
    VMMDLL_MemFree(pid_list);
  }
  return ret_pid_list;
}

PROCESS_INFORMATION ProcessGetInformation(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid) {
  return ProcessGetInformationAll(handle).at(pid);
}

}  // namespace VMM
