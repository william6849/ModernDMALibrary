#include "device.h"

#include "spdlog/spdlog.h"
#include "vmmdll.h"

Device& Device::GetInstance() {
  static Device instance;
  return instance;
}

Device::Device() {}

bool Device::InitDevice(std::string process_path) {
  LPCSTR* ss;
  std::vector<std::string> command = {process_path, "-device", "fpga://algo=0"};
  static auto vmm_handle = VMMDLL_Initialize(command.size(), ss);
  return false;
}
