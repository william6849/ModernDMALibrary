#include "dmaio.h"

#include <chrono>
#include <expected>
#include <future>
#include <span>

#include "io_proc.h"
#include "spdlog/spdlog.h"

DMAIO::DMAIO() : dma_exec_(std::make_shared<DMATaskExecutor>()) {}

DMAIO::DMAIO(const std::string& params)
    : dma_exec_(std::make_shared<DMATaskExecutor>()) {
  this->Init(params);
}

const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> DMAIO::vmm_handle() const {
  return dma_exec_->vmm_handle();
}

const std::shared_ptr<DMATaskExecutor> DMAIO::GetExecutor() {
  return dma_exec_;
}

void DMAIO::Reset(const std::string& params) { this->Init(params); }

void DMAIO::Init(const std::string& params) {
  spdlog::info("Initializing VMM");
  constexpr int32_t vmm_init_timeout = 60;

  auto async_vmm_init =
      std::async(std::launch::async, [&]() { return VMM::Initialize(params); });

  if (async_vmm_init.wait_for(std::chrono::seconds(vmm_init_timeout)) ==
      std::future_status::timeout) {
    spdlog::critical("VMM Initialize timeout, retry or reboot target fully.");
    exit(0);
  }

  auto vmm_handle = async_vmm_init.get();
  if (vmm_handle == nullptr) {
    spdlog::error("VMM Initialize Failed.");
    exit(0);
  }

  dma_exec_->SetIOHandler(vmm_handle);

  spdlog::debug("VMMDLL_Initialize return address: {}",
                static_cast<void*>(vmm_handle));

  spdlog::info("Initializing LC");
  LC_CONFIG lc_config = {.dwVersion = LC_CONFIG_VERSION,
                         .szDevice = "existing"};
  auto leechcore_handler = LcCreate(&lc_config);
  if (!leechcore_handler) {
    throw std::runtime_error("LcCreate failed.");
  }

  constexpr uint8_t value_high = 0x10;
  constexpr uint8_t value_low = 0x00;
  constexpr uint8_t mask_high = 0x10;
  constexpr uint8_t mask_low = 0x00;
  std::array<BYTE, 4> cmd = {value_high, value_low, mask_high, mask_low};
  std::span<BYTE> span = cmd;

  LcCommand(leechcore_handler, LC_CMD_FPGA_CFGREGPCIE_MARKWR | 0x002, 4,
            span.data(), nullptr, nullptr);
  dma_exec_->SetIOHandler(leechcore_handler);

  spdlog::info("Device IO initialized");
}

std::future<std::optional<std::vector<uint8_t>>> DMAIO::Read(
    uint32_t pid, uint64_t virtual_addr, size_t bytes) const {
  return dma_exec_->VMMCall(VMM::MemReadEx, pid, virtual_addr, bytes, 0);
}

std::future<bool> DMAIO::Write(int32_t pid, uint64_t virtual_addr,
                               std::vector<uint8_t>& data) const {
  return dma_exec_->VMMCall(VMM::MemWrite, pid, virtual_addr, data);
}

std::future<uint32_t> DMAIO::ReadScatter(Scatter& scatter) {
  auto scatter_list = std::vector<PMEM_SCATTER>(scatter.SRPMap().size());
  for (auto [addr, srp] : scatter.SRPMap()) {
    scatter_list.push_back(&srp.scatter);
  }
  auto success_entries = dma_exec_->VMMCall(
      VMM::MemReadScatter, scatter.pid(), scatter_list.data(),
      scatter_list.size(), scatter.flags());
  return success_entries;
}

std::future<uint32_t> DMAIO::WriteScatter(Scatter& scatter) {
  auto scatter_list = std::vector<PMEM_SCATTER>(scatter.SRPMap().size());
  for (auto [addr, srp] : scatter.SRPMap()) {
    scatter_list.push_back(&srp.scatter);
  }
  auto success_entries =
      dma_exec_->VMMCall(VMM::MemWriteScatter, scatter.pid(),
                         scatter_list.data(), scatter_list.size());
  return success_entries;
}
