#include "device.h"

#include <sstream>
#include <stdexcept>
#include <vector>

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
