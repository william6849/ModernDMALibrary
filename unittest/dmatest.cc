#include "device_manager.h"
#include "gtest/gtest.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "spdlog/spdlog.h"

TEST(DMATest, device_init) {
  spdlog::set_level(spdlog::level::debug);
  auto& devm = DeviceManager::GetInstance();

  std::string params = "-device /home/zznzm/dump.dmp";

  auto ret = devm.OpenDevice(params);
  auto& dev = devm.GetDevice(ret);

  dev.options.CORE_VERBOSE = 1;
  auto what = dev.Read(0x1000, 0x200);
  while (what.wait_for(std::chrono::microseconds(0)) ==
         std::future_status::timeout) {
  }
  auto ok = what.get();
  spdlog::debug("VMM::Read at 0x{:x}: {}", 0x1000, spdlog::to_hex(ok));

  // multi thread read test
  std::vector<std::thread> tasks;
  for (uint8_t i = 0; i < 10; ++i) {
    tasks.emplace_back([dev, i]() {
      auto addr = 0x80000 * i;
      auto ret = dev.Read(addr, 0x200).get();  // blocking
      spdlog::debug("VMM::Read at 0x{:x}: {}", addr, spdlog::to_hex(ret));
    });
    spdlog::debug("goed?");
  }
  spdlog::debug("Non block?");
  for (uint8_t i = 0; i < 10; ++i) {
    tasks.at(i).join();
  }

  // auto last_ret_data = ret_data;
  //  std::vector<uint8_t> writebytes = {0x11, 0x22, 0x33};
  //  dev.Write(0x1000, writebytes);
  //  spdlog::debug("VMM::Write at {:x}: {}", addr, spdlog::to_hex(data));

  // ret_data = dev.Read(0x1000, 0x250);
  // dev.Write(0x1000, last_ret_data);

  spdlog::debug("opt2: {}", (uint64_t)dev.options.CORE_VERBOSE);
}
