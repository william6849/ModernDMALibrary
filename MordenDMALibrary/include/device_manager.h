#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <cstdint>
#include <string>
#include <vector>

#include "device.h"

class DeviceManager {
 public:
  static DeviceManager& GetInstance();
  Device& GetDevice(int32_t num);

  int32_t OpenDevice(const std::string& params);
  const std::vector<Device>& device_list() const;

 private:
  DeviceManager();
  DeviceManager(const DeviceManager&) = delete;
  void operator=(const DeviceManager&) = delete;

  std::vector<Device> device_list_;
};

#endif
