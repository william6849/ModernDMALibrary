#include "dmaio.h"

#include <chrono>
#include <future>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "spdlog/spdlog.h"

static VMM_HANDLE hVMM_ = nullptr;

template <typename S>
OptionProxy<S>::OptionProxy(uint64_t opt, bool read, bool write)
    : opt_(opt), read_(read), write_(write){};

template <typename S>
S OptionProxy<S>::Read() {
  if (!read_) {
    spdlog::error("Invalid read access: {}", __func__);
  }
  if (!VMMDLL_ConfigGet(hVMM_, static_cast<ULONG64>(opt_),
                        reinterpret_cast<PULONG64>(&value_))) {
    spdlog::error("VMMDLL_ConfigGet Failed");
  }
  return value_;
}

template <typename S>
void OptionProxy<S>::Write(const uint64_t& val) {
  if (!write_) {
    spdlog::error("Invalid write access");
  }
  if (!VMMDLL_ConfigSet(hVMM_, static_cast<ULONG64>(opt_),
                        static_cast<ULONG64>(val))) {
    spdlog::error("VMMDLL_ConfigSet Failed");
  }
  value_ = static_cast<S>(val);
}

DMAIO::DMAIO() {}

DMAIO::DMAIO(const std::string& params) { this->Init(params); }

DMAIO::operator VMM_HANDLE() const { return vmm_handle_.get(); }

const VMM_HANDLE DMAIO::vmm_handle() const { return vmm_handle_.get(); }

void DMAIO::Reset(const std::string& params) { this->Init(params); }

void DMAIO::Init(const std::string& params) {
  spdlog::info("Initializing VMM");

  auto async_vmm_init =
      std::async(std::launch::async, [&]() { return VMM::Initialize(params); });

  if (async_vmm_init.wait_for(std::chrono::seconds(60)) ==
      std::future_status::timeout) {
    spdlog::critical("VMM Initialize timeout, retry or reboot target fully.");
    exit(0);
  }

  auto vmm_handle = async_vmm_init.get();
  if (vmm_handle == nullptr) {
    spdlog::error("VMM Initialize Failed.");
    exit(0);
  }
  hVMM_ = vmm_handle;
  vmm_handle_.reset(vmm_handle, VMMHandleDeleter);

  spdlog::debug("VMMDLL_Initialize return address: {}",
                static_cast<void*>(vmm_handle));

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
  lc_handle_.reset(leechcore_handler, LCHandleDeleter);

  spdlog::info("Device IO initialized");
}

std::vector<uint8_t> DMAIO::Read(uint64_t physical_addr, size_t bytes) const {
  return VMM::MemReadEx(vmm_handle(), -1, physical_addr, bytes, 0);
}

bool DMAIO::Write(uint64_t physical_addr, std::vector<uint8_t>& data) const {
  return VMM::MemWrite(vmm_handle(), -1, physical_addr, data);
}

std::vector<uint8_t> DMAIO::Read(uint32_t pid, uint64_t virtual_addr,
                                 size_t bytes) const {
  return VMM::MemReadEx(vmm_handle(), pid, virtual_addr, bytes, 0);
}

bool DMAIO::Write(uint32_t pid, uint64_t virtual_addr,
                  std::vector<uint8_t>& data) const {
  return VMM::MemWrite(vmm_handle(), pid, virtual_addr, data);
}

namespace Target {
Options::Options()
    : CORE_PRINTF_ENABLE(VMMDLL_OPT_CORE_PRINTF_ENABLE, true, true),
      CORE_VERBOSE(VMMDLL_OPT_CORE_VERBOSE, true, true),
      CORE_VERBOSE_EXTRA(VMMDLL_OPT_CORE_VERBOSE_EXTRA, true, true),
      CORE_VERBOSE_EXTRA_TLP(VMMDLL_OPT_CORE_VERBOSE_EXTRA_TLP, true, true),
      CORE_MAX_NATIVE_ADDRESS(VMMDLL_OPT_CORE_MAX_NATIVE_ADDRESS, true, false),
      CORE_LEECHCORE_HANDLE(VMMDLL_OPT_CORE_LEECHCORE_HANDLE, true, false),
      CORE_VMM_ID(VMMDLL_OPT_CORE_VMM_ID, true, false),
      CORE_SYSTEM(VMMDLL_OPT_CORE_SYSTEM, true, false),
      CORE_MEMORYMODEL(VMMDLL_OPT_CORE_MEMORYMODEL, true, false),
      CONFIG_IS_REFRESH_ENABLED(VMMDLL_OPT_CONFIG_IS_REFRESH_ENABLED, true,
                                false),
      CONFIG_TICK_PERIOD(VMMDLL_OPT_CONFIG_TICK_PERIOD, true, true),
      CONFIG_READCACHE_TICKS(VMMDLL_OPT_CONFIG_READCACHE_TICKS, true, true),
      CONFIG_TLBCACHE_TICKS(VMMDLL_OPT_CONFIG_TLBCACHE_TICKS, true, true),
      CONFIG_PROCCACHE_TICKS_PARTIAL(VMMDLL_OPT_CONFIG_PROCCACHE_TICKS_PARTIAL,
                                     true, true),
      CONFIG_PROCCACHE_TICKS_TOTAL(VMMDLL_OPT_CONFIG_PROCCACHE_TICKS_TOTAL,
                                   true, true),
      CONFIG_VMM_VERSION_MAJOR(VMMDLL_OPT_CONFIG_VMM_VERSION_MAJOR, true,
                               false),
      CONFIG_VMM_VERSION_MINOR(VMMDLL_OPT_CONFIG_VMM_VERSION_MINOR, true,
                               false),
      CONFIG_VMM_VERSION_REVISION(VMMDLL_OPT_CONFIG_VMM_VERSION_REVISION, true,
                                  false),
      CONFIG_STATISTICS_FUNCTIONCALL(VMMDLL_OPT_CONFIG_STATISTICS_FUNCTIONCALL,
                                     true, true),
      CONFIG_IS_PAGING_ENABLED(VMMDLL_OPT_CONFIG_IS_PAGING_ENABLED, true, true),
      CONFIG_DEBUG(VMMDLL_OPT_CONFIG_DEBUG, false, true),
      CONFIG_YARA_RULES(VMMDLL_OPT_CONFIG_YARA_RULES, true, false),
      WIN_VERSION_MAJOR(VMMDLL_OPT_WIN_VERSION_MAJOR, true, false),
      WIN_VERSION_MINOR(VMMDLL_OPT_WIN_VERSION_MINOR, true, false),
      WIN_VERSION_BUILD(VMMDLL_OPT_WIN_VERSION_BUILD, true, false),
      WIN_SYSTEM_UNIQUE_ID(VMMDLL_OPT_WIN_SYSTEM_UNIQUE_ID, true, false),
      FORENSIC_MODE(VMMDLL_OPT_FORENSIC_MODE, true, false),
      REFRESH_ALL(VMMDLL_OPT_REFRESH_ALL, false, true),
      REFRESH_FREQ_MEM(VMMDLL_OPT_REFRESH_FREQ_MEM, false, true),
      REFRESH_FREQ_MEM_PARTIAL(VMMDLL_OPT_REFRESH_FREQ_MEM_PARTIAL, false,
                               true),
      REFRESH_FREQ_TLB(VMMDLL_OPT_REFRESH_FREQ_TLB, false, true),
      REFRESH_FREQ_TLB_PARTIAL(VMMDLL_OPT_REFRESH_FREQ_TLB_PARTIAL, false,
                               true),
      REFRESH_FREQ_FAST(VMMDLL_OPT_REFRESH_FREQ_FAST, false, true),
      REFRESH_FREQ_MEDIUM(VMMDLL_OPT_REFRESH_FREQ_MEDIUM, false, true),
      REFRESH_FREQ_SLOW(VMMDLL_OPT_REFRESH_FREQ_SLOW, false, true),
      PROCESS_DTB(VMMDLL_OPT_PROCESS_DTB, false, true),
      PROCESS_DTB_FAST_LOWINTEGRITY(VMMDLL_OPT_PROCESS_DTB_FAST_LOWINTEGRITY,
                                    false, true) {}
}  // namespace Target
