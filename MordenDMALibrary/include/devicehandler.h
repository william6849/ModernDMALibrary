#ifndef DEVICE_HANDLER_H
#define DEVICE_HANDLER_H

#include <string>
#include <vector>

#include "device.h"

class DeviceHandler {
 public:
  static DeviceHandler& GetInstance();
  DeviceHandler(const DeviceHandler&) = delete;
  void operator=(const DeviceHandler&) = delete;
  // virtual ~DeviceHandler();

  bool AddDevice(std::string device_path);
  const std::vector<Device>& device_list() const;

 private:
  DeviceHandler();
  std::vector<Device> device_list_;
};

#endif
