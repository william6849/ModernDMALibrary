#include "include/memory.h"

#include "LeechCore/includes/vmmdll.h"
#include "spdlog/spdlog.h"

MemoryControl& MemoryControl::GetInstance() {
  static MemoryControl instance;
  return instance;
}

MemoryControl::MemoryControl() {}

bool MemoryControl::InitDevice(std::string process_path) {
  std::string command = {process_path, "-device", "fpga://algo=0"};
  static auto vmm_handle = VMMDLL_Initialize();
  return false;
}
