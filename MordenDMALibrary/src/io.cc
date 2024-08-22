#include "io.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "spdlog/fmt/bin_to_hex.h"
#include "spdlog/spdlog.h"

template <typename Func, typename... Args>
static auto AsyncWithTimeout(Func&& func, std::chrono::milliseconds timeout,
                             Args&&... args) {
  using ReturnType = decltype(func(std::forward<Args>(args)...));

  auto future = std::async(std::launch::async, std::forward<Func>(func),
                           std::forward<Args>(args)...);

  if (future.wait_for(timeout) == std::future_status::ready) {
    return future.get();
  } else {
    throw std::runtime_error("Function call timed out");
  }
}

DMAIO::DMAIO() {};

DMAIO::DMAIO(const std::string& params) { this->Init(params); }

DMAIO::operator VMM_HANDLE() const { return vmm_handle_.get(); }

const VMM_HANDLE DMAIO::vmm_handle() const { return vmm_handle_.get(); }

void DMAIO::Reset(const std::string& params) { this->Init(params); }

void DMAIO::Init(const std::string& params) {
  spdlog::debug("Initializing VMM with parameters: {}", params);
  std::stringstream ss(params);

  std::vector<std::string> parsed;
  std::string token;
  while (getline(ss, token, ' ')) {
    parsed.push_back(token);
  }

  std::vector<LPCSTR> c_str_vec;
  c_str_vec.reserve(parsed.size());

  for (const auto& param : parsed) {
    c_str_vec.push_back(param.c_str());
  }

  vmm_handle_.reset(nullptr);
  auto hVMM = VMMDLL_Initialize(c_str_vec.size(), c_str_vec.data());
  spdlog::debug("VMMDLL_Initialize return: {}", static_cast<void*>(hVMM));
  if (hVMM) {
    vmm_handle_.reset(hVMM);
  } else {
    throw std::runtime_error(
        "VMM operation failed(memo to make custom VMM exceptions)");
  }
}

std::vector<uint8_t> DMAIO::Read(uint64_t addr, size_t bytes) const {
  std::vector<uint8_t> ret(bytes);
  auto result =
      AsyncWithTimeout(VMMDLL_MemRead, std::chrono::milliseconds(5000),
                       vmm_handle(), -1, addr, ret.data(), bytes);
  if (result == 0) {
    throw std::runtime_error("MemRead error");
  }
  spdlog::debug("DMAIO::Read at {:x}: {}", addr, spdlog::to_hex(ret));
  return ret;
}

bool DMAIO::Write(uint64_t addr, std::vector<uint8_t> data) const {
  auto result =
      AsyncWithTimeout(VMMDLL_MemWrite, std::chrono::milliseconds(5000),
                       vmm_handle(), -1, addr, data.data(), data.size());
  if (result == false) {
    throw std::runtime_error("MemWrite error");
  }
  spdlog::debug("DMAIO::Write at {:x}: {}", addr, spdlog::to_hex(data));
  return result;
}
