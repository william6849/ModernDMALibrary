#include "devicehandler.h"

#include "device.h"
#include "spdlog/spdlog.h"

DeviceHandler& DeviceHandler::GetInstance() {
  static DeviceHandler inst;
  return inst;
}

DeviceHandler::DeviceHandler() : device_list_() {}

bool DeviceHandler::AddDevice(std::string process_path) {
  try {
    Device dev(process_path);
    device_list_.push_back(std::move(dev));
    return true;
  } catch (const std::invalid_argument& e) {
    return false;
  }
}

const std::vector<Device>& DeviceHandler::device_list() const {
  return device_list_;
}
