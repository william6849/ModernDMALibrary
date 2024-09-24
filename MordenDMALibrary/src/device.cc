#include "device.h"

Device::Device(const std::string& params) : io_(params), options(nullptr) {};

std::vector<uint8_t> Device::Read(uint64_t addr, size_t bytes) const {
  return io_.Read(addr, bytes);
}

bool Device::Write(uint64_t addr, std::vector<uint8_t> data) const {
  return io_.Write(addr, data);
}
