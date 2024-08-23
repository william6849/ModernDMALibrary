#include "device_manager.h"
#include "gtest/gtest.h"
#include "spdlog/spdlog.h"

TEST(DMATest, device_init) {
  spdlog::set_level(spdlog::level::debug);
  auto& devm = DeviceManager::GetInstance();
  // This should be false cuz we don't have mock on VMM IO nor stable DMA
  // envirement(still can be test on your target).
  auto params =
      "-device fpga -memmap "
      "/home/zznzm/repos/MordenDMALibrary/dump.txt";

  if (ret < 0) return;
  auto& dev = DeviceManager::GetDevice(ret);
  auto ret_data = dev.Read(0x1000, 16);
  auto last_ret_data = ret_data;
  std::vector<uint8_t> writebytes = {0x11, 0x22, 0x33};
  dev.Write(0x1000, writebytes);

  ret_data = dev.Read(0x1000, 16);
  dev.Write(0x1000, last_ret_data);
}
