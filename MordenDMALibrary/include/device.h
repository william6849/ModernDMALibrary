#ifndef DEVICE_H
#define DEVICE_H

#include <cstdint>
#include <string>
#include <vector>

#include "dmaio.h"

class Device {
 public:
  explicit Device(const std::string& params);
  Device(Device&& other) noexcept;
  Device& operator=(Device&& other) noexcept;

  std::vector<uint8_t> Read(uint64_t addr, size_t bytes) const;
  bool Write(uint64_t addr, std::vector<uint8_t> data) const;

  Target::Options options;

 private:
  Device(const Device&) = delete;
  Device& operator=(const Device&) = delete;

  DMAIO io_;
};

#endif
