#include "dmaio.h"

#include <chrono>
#include <future>
#include <span>
#include <stdexcept>

#include "io_proc.h"
#include "spdlog/spdlog.h"

template <typename S>
OptionProxy<S>::OptionProxy(const std::shared_ptr<DMATaskExecutor>& dma_exec,
                            uint64_t opt, bool read, bool write)
    : dma_exec_(dma_exec), opt_(opt), read_(read), write_(write){};

template <typename S>
S OptionProxy<S>::Read() {
  if (!read_) {
    spdlog::error("Invalid read access: {}", __func__);
  }

  if (!dma_exec_.lock()
           ->VMMCall(VMMDLL_ConfigGet, static_cast<ULONG64>(opt_),
                     reinterpret_cast<PULONG64>(&value_))
           .get()) {
    spdlog::error("VMMDLL_ConfigGet Failed");
  }
  return value_;
}

template <typename S>
void OptionProxy<S>::Write(const uint64_t& val) {
  if (!write_) {
    spdlog::error("Invalid write access");
  }

  if (!dma_exec_.lock()
           ->VMMCall(VMMDLL_ConfigSet, static_cast<ULONG64>(opt_),
                     static_cast<ULONG64>(val))
           .get()) {
    spdlog::error("VMMDLL_ConfigSet Failed");
  }
  value_ = static_cast<S>(val);
}

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

std::future<std::vector<uint8_t>> DMAIO::Read(uint32_t pid,
                                              uint64_t virtual_addr,
                                              size_t bytes) const {
  return dma_exec_->VMMCall(VMM::MemReadEx, pid, virtual_addr, bytes, 0);
}

std::future<bool> DMAIO::Write(int32_t pid, uint64_t virtual_addr,
                               const std::vector<uint8_t>& data) const {
  return dma_exec_->VMMCall(VMM::MemWrite, pid, virtual_addr, data);
}

namespace Target {
Options::Options(const std::shared_ptr<DMATaskExecutor> dma_exec)
    : CORE_PRINTF_ENABLE(dma_exec, VMMDLL_OPT_CORE_PRINTF_ENABLE, true, true),
      CORE_VERBOSE(dma_exec, VMMDLL_OPT_CORE_VERBOSE, true, true),
      CORE_VERBOSE_EXTRA(dma_exec, VMMDLL_OPT_CORE_VERBOSE_EXTRA, true, true),
      CORE_VERBOSE_EXTRA_TLP(dma_exec, VMMDLL_OPT_CORE_VERBOSE_EXTRA_TLP, true,
                             true),
      CORE_MAX_NATIVE_ADDRESS(dma_exec, VMMDLL_OPT_CORE_MAX_NATIVE_ADDRESS,
                              true, false),
      CORE_LEECHCORE_HANDLE(dma_exec, VMMDLL_OPT_CORE_LEECHCORE_HANDLE, true,
                            false),
      CORE_VMM_ID(dma_exec, VMMDLL_OPT_CORE_VMM_ID, true, false),
      CORE_SYSTEM(dma_exec, VMMDLL_OPT_CORE_SYSTEM, true, false),
      CORE_MEMORYMODEL(dma_exec, VMMDLL_OPT_CORE_MEMORYMODEL, true, false),
      CONFIG_IS_REFRESH_ENABLED(dma_exec, VMMDLL_OPT_CONFIG_IS_REFRESH_ENABLED,
                                true, false),
      CONFIG_TICK_PERIOD(dma_exec, VMMDLL_OPT_CONFIG_TICK_PERIOD, true, true),
      CONFIG_READCACHE_TICKS(dma_exec, VMMDLL_OPT_CONFIG_READCACHE_TICKS, true,
                             true),
      CONFIG_TLBCACHE_TICKS(dma_exec, VMMDLL_OPT_CONFIG_TLBCACHE_TICKS, true,
                            true),
      CONFIG_PROCCACHE_TICKS_PARTIAL(
          dma_exec, VMMDLL_OPT_CONFIG_PROCCACHE_TICKS_PARTIAL, true, true),
      CONFIG_PROCCACHE_TICKS_TOTAL(
          dma_exec, VMMDLL_OPT_CONFIG_PROCCACHE_TICKS_TOTAL, true, true),
      CONFIG_VMM_VERSION_MAJOR(dma_exec, VMMDLL_OPT_CONFIG_VMM_VERSION_MAJOR,
                               true, false),
      CONFIG_VMM_VERSION_MINOR(dma_exec, VMMDLL_OPT_CONFIG_VMM_VERSION_MINOR,
                               true, false),
      CONFIG_VMM_VERSION_REVISION(
          dma_exec, VMMDLL_OPT_CONFIG_VMM_VERSION_REVISION, true, false),
      CONFIG_STATISTICS_FUNCTIONCALL(
          dma_exec, VMMDLL_OPT_CONFIG_STATISTICS_FUNCTIONCALL, true, true),
      CONFIG_IS_PAGING_ENABLED(dma_exec, VMMDLL_OPT_CONFIG_IS_PAGING_ENABLED,
                               true, true),
      CONFIG_DEBUG(dma_exec, VMMDLL_OPT_CONFIG_DEBUG, false, true),
      CONFIG_YARA_RULES(dma_exec, VMMDLL_OPT_CONFIG_YARA_RULES, true, false),
      WIN_VERSION_MAJOR(dma_exec, VMMDLL_OPT_WIN_VERSION_MAJOR, true, false),
      WIN_VERSION_MINOR(dma_exec, VMMDLL_OPT_WIN_VERSION_MINOR, true, false),
      WIN_VERSION_BUILD(dma_exec, VMMDLL_OPT_WIN_VERSION_BUILD, true, false),
      WIN_SYSTEM_UNIQUE_ID(dma_exec, VMMDLL_OPT_WIN_SYSTEM_UNIQUE_ID, true,
                           false),
      FORENSIC_MODE(dma_exec, VMMDLL_OPT_FORENSIC_MODE, true, false),
      REFRESH_ALL(dma_exec, VMMDLL_OPT_REFRESH_ALL, false, true),
      REFRESH_FREQ_MEM(dma_exec, VMMDLL_OPT_REFRESH_FREQ_MEM, false, true),
      REFRESH_FREQ_MEM_PARTIAL(dma_exec, VMMDLL_OPT_REFRESH_FREQ_MEM_PARTIAL,
                               false, true),
      REFRESH_FREQ_TLB(dma_exec, VMMDLL_OPT_REFRESH_FREQ_TLB, false, true),
      REFRESH_FREQ_TLB_PARTIAL(dma_exec, VMMDLL_OPT_REFRESH_FREQ_TLB_PARTIAL,
                               false, true),
      REFRESH_FREQ_FAST(dma_exec, VMMDLL_OPT_REFRESH_FREQ_FAST, false, true),
      REFRESH_FREQ_MEDIUM(dma_exec, VMMDLL_OPT_REFRESH_FREQ_MEDIUM, false,
                          true),
      REFRESH_FREQ_SLOW(dma_exec, VMMDLL_OPT_REFRESH_FREQ_SLOW, false, true),
      PROCESS_DTB(dma_exec, VMMDLL_OPT_PROCESS_DTB, false, true),
      PROCESS_DTB_FAST_LOWINTEGRITY(
          dma_exec, VMMDLL_OPT_PROCESS_DTB_FAST_LOWINTEGRITY, false, true) {}
}  // namespace Target
