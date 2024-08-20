#include "devicehandler.h"
#include "gtest/gtest.h"

TEST(DMATest, device_init) {
  auto& dev = DeviceHandler::GetInstance();
  // This should be false cuz we don't have mock on VMM IO nor stable DMA
  // envirement(still can be test on your target).
  EXPECT_FALSE(dev.AddDevice("-device fpga"));
  auto& dev_list = dev.device_list();
}
