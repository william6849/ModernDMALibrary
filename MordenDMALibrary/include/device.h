#ifndef DEVICE_H
#define DEVICE_H

#include <cstdint>
#include <string>
#include <vector>

#include "dmaio.h"

class Device {
 public:
  explicit Device(const std::string& params);

  auto Read(uint64_t addr, std::size_t bytes) const
      -> std::invoke_result_t<decltype(&DMAIO::Read), const DMAIO, uint32_t,
                              uint64_t, std::size_t>;

  auto Write(uint64_t addr, std::vector<uint8_t>& data) const
      -> std::invoke_result_t<decltype(&DMAIO::Write), const DMAIO, uint32_t,
                              uint64_t, std::vector<uint8_t>&>;

  Target::Options options;

 private:
  DMAIO io_;
};

#endif
