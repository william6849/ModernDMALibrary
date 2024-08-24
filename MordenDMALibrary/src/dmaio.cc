#include "dmaio.h"

#include <chrono>
#include <future>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "spdlog/fmt/bin_to_hex.h"
#include "spdlog/spdlog.h"

static VMM_HANDLE hVMM_ = nullptr;

DMAIO::DMAIO(const std::string& params) : vmm_handle_(nullptr) {
  this->Init(params);
}

DMAIO::operator VMM_HANDLE() const { return vmm_handle_.get(); }

const VMM_HANDLE DMAIO::vmm_handle() const { return vmm_handle_.get(); }

void DMAIO::Reset(const std::string& params) { this->Init(params); }

void DMAIO::Init(const std::string& params) {
  spdlog::info("Initializing VMM");
  spdlog::debug("Parameters: {}", params);

  std::stringstream params_stream(params);
  std::vector<std::string> params_parsed;
  std::string token;
  while (getline(params_stream, token, ' ')) {
    params_parsed.push_back(token);
  }

  std::vector<LPCSTR> c_str_vec;
  for (const auto& param : params_parsed) {
    c_str_vec.push_back(param.c_str());
  }

  auto async_vmm_init = std::async(std::launch::async, [&]() {
    return VMMDLL_Initialize(c_str_vec.size(), c_str_vec.data());
  });
  if (async_vmm_init.wait_for(std::chrono::seconds(60)) ==
      std::future_status::timeout) {
    spdlog::critical(
        "VMMDLL_Initialize timeout, retry or reboot target fully.");
    exit(0);
  }

  auto vmm_handle = async_vmm_init.get();
  if (vmm_handle == nullptr) {
    spdlog::error("VMMDLL_Initialize Failed.");
    exit(0);
  }
  hVMM_ = vmm_handle;
  vmm_handle_.reset(vmm_handle);

  spdlog::debug("VMMDLL_Initialize return: {}", static_cast<void*>(vmm_handle));

  spdlog::info("Initializing LC");
  LC_CONFIG lc_config = {.dwVersion = LC_CONFIG_VERSION,
                         .szDevice = "existing"};
  auto leechcore_handler = LcCreate(&lc_config);
  if (!leechcore_handler) {
    throw std::runtime_error("LcCreate failed.");
  }
  BYTE cmd[4] = {0x10, 0x00, 0x10, 0x00};
  LcCommand(leechcore_handler, LC_CMD_FPGA_CFGREGPCIE_MARKWR | 0x002, 4, cmd,
            NULL, NULL);
  lc_handle_.reset(leechcore_handler);
  spdlog::info("Device IO initialized");
}

std::vector<uint8_t> DMAIO::Read(uint64_t addr, size_t bytes) const {
  std::vector<uint8_t> ret(bytes);
  auto result = VMMDLL_MemRead(vmm_handle(), static_cast<DWORD>(-1), addr,
                               ret.data(), static_cast<DWORD>(bytes));
  if (result == 0) {
    throw std::runtime_error("MemRead error");
  }
  spdlog::debug("DMAIO::Read at {:x}: {}", addr, spdlog::to_hex(ret));
  return ret;
}

bool DMAIO::Write(uint64_t addr, std::vector<uint8_t> data) const {
  auto result = VMMDLL_MemWrite(vmm_handle(), static_cast<DWORD>(-1), addr,
                                data.data(), static_cast<DWORD>(data.size()));
  if (result == false) {
    throw std::runtime_error("MemWrite error");
  }
  spdlog::debug("DMAIO::Write at {:x}: {}", addr, spdlog::to_hex(data));
  return result;
}
