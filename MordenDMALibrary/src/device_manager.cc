#include "device_manager.h"

#include <stdexcept>

#include "spdlog/spdlog.h"

DeviceManager& DeviceManager::GetInstance() {
  static DeviceManager inst;
  return inst;
}

const Device& DeviceManager::GetDevice(int32_t num) {
  return DeviceManager::GetInstance().device_list().at(num);
}

DeviceManager::DeviceManager() : device_list_() {}

int32_t DeviceManager::OpenDevice(const std::string& params) {
  try {
    Device dev(params);
    device_list_.push_back(std::move(dev));
    spdlog::info("Device opened.");
    return device_list_.size() - 1;
  } catch (const std::runtime_error& e) {
    spdlog::error("{}", e.what());
    return -1;
  }
}

const std::vector<Device>& DeviceManager::device_list() const {
  return device_list_;
}
