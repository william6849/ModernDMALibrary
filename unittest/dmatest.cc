#include "device_manager.h"
#include "gtest/gtest.h"
#include "spdlog/spdlog.h"

TEST(DMATest, device_init) {
  spdlog::set_level(spdlog::level::debug);
  auto& dev = DeviceManager::GetInstance();
  // This should be false cuz we don't have mock on VMM IO nor stable DMA
  // envirement(still can be test on your target).
  auto params =
      "-device fpga -memmap "
      "/home/zznzm/repos/MordenDMALibrary/dump.txt";
  auto ret = dev.OpenDevice(params);
  if (ret < 0) return;
  auto& dev2 = DeviceManager::GetDevice(ret);
  auto ret2 = dev2.Read(0x1000, 16);

  std::vector<uint8_t> bewrite = {0x11, 0x22, 0x33};
  dev2.Write(0x1000, bewrite);

  ret2 = dev2.Read(0x1000, 16);
}
