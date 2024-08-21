#ifndef DEVICE_H
#define DEVICE_H

#include <string>

#include "leechwrapper.h"

class Device {
 public:
  Device(const std::string& params);

  Device(const Device&) = delete;
  Device& operator=(const Device&) = delete;

  Device(Device&& other) noexcept;

  Device& operator=(Device&& other) noexcept;

 protected:
  void InitVMM(const std::string& params);

 private:
  UniqueVMMHandle vmm_handle_;
};

#endif
