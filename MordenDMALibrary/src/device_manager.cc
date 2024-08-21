#include "device_manager.h"

#include "device.h"
#include "spdlog/spdlog.h"

DeviceManager& DeviceManager::GetInstance() {
  static DeviceManager inst;
  return inst;
}

DeviceManager::DeviceManager() : device_list_() {}

bool DeviceManager::AddDevice(std::string process_path) {
  try {
    Device dev(process_path);
    device_list_.push_back(std::move(dev));
    return true;
  } catch (const std::invalid_argument& e) {
    return false;
  }
}

const std::vector<Device>& DeviceManager::device_list() const {
  return device_list_;
}
