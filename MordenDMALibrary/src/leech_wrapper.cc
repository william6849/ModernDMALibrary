#include "leech_wrapper.h"

#include "spdlog/fmt/bin_to_hex.h"
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
                               const uint64_t addr, const size_t bytes,
                               uint32_t flag) {
  std::vector<uint8_t> ret(bytes);
  uint32_t read_bytes = 0;
  auto result = VMMDLL_MemReadEx(handle, static_cast<DWORD>(pid), addr,
                                 ret.data(), static_cast<DWORD>(bytes),
                                 &read_bytes, static_cast<DWORD>(flag));
  if (result == 0) {
    throw std::runtime_error("MemRead error");
  }
  spdlog::debug("VMM::Read at {:x}: {}", addr, spdlog::to_hex(ret));
  return std::move(ret);
}

bool MemWrite(const VMM_HANDLE handle, const uint32_t pid, const uint64_t addr,
              std::vector<uint8_t>& data) {
  uint32_t write_bytes = 0;
  auto result = VMMDLL_MemWrite(handle, static_cast<DWORD>(pid), addr,
                                data.data(), static_cast<DWORD>(data.size()));
  if (result == false) {
    throw std::runtime_error("MemWrite error");
  }
  spdlog::debug("VMM::Write at {:x}: {}", addr, spdlog::to_hex(data));
  return result;
}
}  // namespace VMM
