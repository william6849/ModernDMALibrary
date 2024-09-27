#include "device.h"

Device::Device(const std::string& params) : io_(params), options(nullptr) {
  options = Target::Options(io_.GetExecutor());
};

auto Device::Read(uint64_t addr, size_t bytes) const
    -> std::invoke_result_t<decltype(&DMAIO::Read), const DMAIO, uint32_t,
                            uint64_t, size_t> {
  return std::invoke(&DMAIO::Read, io_, -1, addr, bytes);
}

auto Device::Write(uint64_t addr, const std::vector<uint8_t>& data) const
    -> std::invoke_result_t<decltype(&DMAIO::Write), const DMAIO, int32_t,
                            uint64_t, std::vector<uint8_t>&> {
  return std::invoke(&DMAIO::Write, io_, static_cast<int32_t>(-1), addr, data);
}
