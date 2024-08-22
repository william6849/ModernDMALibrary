#include "device.h"

#include <stdexcept>

#include "spdlog/fmt/bin_to_hex.h"
#include "spdlog/spdlog.h"

Device::Device(const std::string& params) : io_(params) {};

Device::Device(Device&& other) noexcept : io_(std::move(other.io_)) {}

Device& Device::operator=(Device&& other) noexcept {
  if (this != &other) {
    io_ = std::move(other.io_);
  }
  return *this;
}

std::vector<uint8_t> Device::Read(uint64_t addr, size_t bytes) const {
  return io_.Read(addr, bytes);
}

bool Device::Write(uint64_t addr, std::vector<uint8_t> data) const {
  return io_.Write(addr, data);
}
