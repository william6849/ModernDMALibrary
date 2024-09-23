#include "device_manager.h"
#include "gtest/gtest.h"
#include "spdlog/spdlog.h"

TEST(DMATest, device_init) {
  spdlog::set_level(spdlog::level::debug);
  auto& devm = DeviceManager::GetInstance();

  std::string params = "-device /home/zznzm/dump.dmp";

  auto ret = devm.OpenDevice(params);
  auto& dev = devm.GetDevice(ret);

  // dev.options.CORE_VERBOSE = 1;

  auto ret_data = dev.Read(0x1000, 0x200);

  auto last_ret_data = ret_data;
  // std::vector<uint8_t> writebytes = {0x11, 0x22, 0x33};
  // dev.Write(0x1000, writebytes);

  ret_data = dev.Read(0x1000, 0x250);
  // dev.Write(0x1000, last_ret_data);

  // spdlog::debug("opt2: {}", (uint64_t)dev.options.CORE_VERBOSE);
}
