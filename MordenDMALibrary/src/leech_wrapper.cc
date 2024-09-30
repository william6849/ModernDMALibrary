#include "leech_wrapper.h"

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

std::vector<uint8_t> MemReadEx(const VMM_HANDLE handle, const uint32_t pid,
                               const uint64_t addr, const std::size_t bytes,
                               uint32_t flag) {
  std::vector<uint8_t> ret(bytes);
  DWORD read_bytes = 0;
  auto result = VMMDLL_MemReadEx(
      handle, static_cast<DWORD>(pid), static_cast<ULONG64>(addr),
      static_cast<PBYTE>(ret.data()), static_cast<DWORD>(bytes), &read_bytes,
      static_cast<DWORD>(flag));
  if (result == 0) {
    throw std::runtime_error("MemRead error");
  }
  return std::move(ret);
}

bool MemWrite(const VMM_HANDLE handle, const int32_t pid, const uint64_t addr,
              std::vector<uint8_t>& data) {
  auto result = VMMDLL_MemWrite(
      handle, static_cast<DWORD>(pid), static_cast<ULONG64>(addr),
      static_cast<PBYTE>(data.data()), static_cast<DWORD>(data.size()));
  if (result == false) {
    throw std::runtime_error("MemWrite error");
  }
  return result;
}

uint32_t MemReadScatter(const VMM_HANDLE handle, const uint32_t pid,
                        PPMEM_SCATTER ppmems, int32_t cpmems, int32_t flags) {
  return VMMDLL_MemReadScatter(handle, pid, ppmems, cpmems, flags);
}

uint32_t MemWriteScatter(const VMM_HANDLE handle, const uint32_t pid,
                         PPMEM_SCATTER ppmems, int32_t cpmems) {
  return VMMDLL_MemWriteScatter(handle, pid, ppmems, cpmems);
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
}  // namespace VMM
