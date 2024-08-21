#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <string>
#include <vector>

#include "device.h"

class DeviceManager {
 public:
  static DeviceManager& GetInstance();
  DeviceManager(const DeviceManager&) = delete;
  void operator=(const DeviceManager&) = delete;
  // virtual ~DeviceManager();

  bool AddDevice(std::string device_path);
  const std::vector<Device>& device_list() const;

 private:
  DeviceManager();
  std::vector<Device> device_list_;
};

#endif
