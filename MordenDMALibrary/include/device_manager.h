#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <cstdint>
#include <string>
#include <vector>

#include "device.h"

class DeviceManager {
 public:
  static DeviceManager& GetInstance();
  static const Device& GetDevice(int32_t num);
  DeviceManager(const DeviceManager&) = delete;
  void operator=(const DeviceManager&) = delete;
  // virtual ~DeviceManager();

  int32_t OpenDevice(std::string params);
  const std::vector<Device>& device_list() const;

 private:
  DeviceManager();
  std::vector<Device> device_list_;
};

#endif
