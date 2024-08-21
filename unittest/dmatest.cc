#include "device_manager.h"
#include "gtest/gtest.h"
#include "spdlog/spdlog.h"

TEST(DMATest, device_init) {
  auto& dev = DeviceManager::GetInstance();
  // This should be false cuz we don't have mock on VMM IO nor stable DMA
  // envirement(still can be test on your target).
  auto params =
      "-device fpga -memmap "
      "/home/zznzm/repos/MordenDMALibrary/build/unixlike-gcc-release/"
      "third_party/Source/MemProcFS/dump.txt";
  dev.AddDevice(params);
  auto& dev_list = dev.device_list();
}
