#include "device.h"
#include "gtest/gtest.h"

TEST(HelloTest, test) { EXPECT_TRUE(1 + 1 == 2); }
TEST(DMATest, device_init) { auto& dev = Device::GetInstance(); }
