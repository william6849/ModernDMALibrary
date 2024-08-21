#include "device.h"

#include <sstream>
#include <stdexcept>

#include "spdlog/fmt/bin_to_hex.h"
#include "spdlog/spdlog.h"

Device::Device(const std::string& params) : vmm_handle_(nullptr) {
  InitVMM(params);
};

Device::Device(Device&& other) noexcept
    : vmm_handle_(std::move(other.vmm_handle_)) {}

Device& Device::operator=(Device&& other) noexcept {
  if (this != &other) {
    vmm_handle_ = std::move(other.vmm_handle_);
  }
  return *this;
}

Device::operator VMM_HANDLE() const { return vmm_handle_.get(); }

const VMM_HANDLE Device::vmm_handle() const { return vmm_handle_.get(); }

void Device::InitVMM(const std::string& params) {
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

  auto hVMM = VMMDLL_Initialize(c_str_vec.size(), c_str_vec.data());

  if (hVMM) {
    vmm_handle_.reset(hVMM);
  } else {
    throw std::runtime_error(
        "VMM operation failed(memo to make custom VMM exceptions)");
  }
}

std::vector<uint8_t> Device::Read(uint64_t addr, size_t bytes) const {
  std::vector<uint8_t> ret(bytes);
  auto result = VMMDLL_MemRead(vmm_handle(), -1, addr, ret.data(), bytes);
  if (result == 0) {
    throw std::runtime_error("MemRead error");
    return std::vector<uint8_t>();
  }
  spdlog::debug("Device::Read at {:x}: {}", addr, spdlog::to_hex(ret));
  return ret;
}

bool Device::Write(uint64_t addr, std::vector<uint8_t> data) const {
  auto result =
      VMMDLL_MemWrite(vmm_handle(), -1, addr, data.data(), data.size());
  if (result == false) {
    throw std::runtime_error("MemWrite error");
  }
  spdlog::debug("Device::Write at {:x}: {}", addr, spdlog::to_hex(data));
  return result;
}
