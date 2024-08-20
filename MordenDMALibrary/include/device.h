#ifndef DEVICE_H
#define DEVICE_H

#include <string>

#include "leechwrapper.h"

class Device {
 public:
  Device(std::string path);

  Device(const Device&) = delete;
  Device& operator=(const Device&) = delete;

  Device(Device&& other) noexcept : vmm_handle_(std::move(other.vmm_handle_)) {}

  Device& operator=(Device&& other) noexcept {
    if (this != &other) {
      vmm_handle_ = std::move(other.vmm_handle_);
    }
    return *this;
  }

 private:
  UniqueVMMHandle vmm_handle_;
};

#endif
