#ifndef DEVICE_H
#define DEVICE_H

#include <cstdint>
#include <string>
#include <vector>

#include "leech_wrapper.h"

class Device {
 public:
  Device(const std::string& params);

  Device(const Device&) = delete;
  Device& operator=(const Device&) = delete;

  Device(Device&& other) noexcept;

  Device& operator=(Device&& other) noexcept;

  operator VMM_HANDLE() const;
  const VMM_HANDLE vmm_handle() const;

  std::vector<uint8_t> Read(uint64_t addr, size_t bytes) const;
  bool Write(uint64_t addr, std::vector<uint8_t> data) const;

 protected:
  void InitVMM(const std::string& params);

 private:
  UniqueVMMHandle vmm_handle_;
};

#endif
