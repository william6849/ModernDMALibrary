#include "device.h"

#include <sstream>

#include "LeechCore/includes/vmmdll.h"
#include "spdlog/spdlog.h"

Device& Device::GetInstance() {
  static Device instance;
  return instance;
}

Device::Device() {}

bool Device::InitDevice(std::string process_path) {
  std::vector<std::string> command = {process_path, "-device", "fpga://algo=0"};
  // static auto vmm_handle =
  //     VMMDLL_Initialize(command.size(), static_cast<>(&command[0]));
  return false;
}