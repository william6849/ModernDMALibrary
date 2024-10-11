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
  uint64_t read_val = 0;
  if (!dma_exec_.lock()->VMMCall(VMM::ConfigGet, opt_, read_val).get()) {
    spdlog::error("VMMDLL_ConfigGet Failed");
  }
  return value_ = static_cast<decltype(value_)>(read_val);
}

template <typename S>
void OptionProxy<S>::Write(const uint64_t& val) {
  if (!write_) {
    spdlog::error("Invalid write access");
  }

  if (!dma_exec_.lock()->VMMCall(VMM::ConfigSet, opt_, val).get()) {
    spdlog::error("VMMDLL_ConfigSet Failed");
  }
  value_ = static_cast<S>(val);
}

Scatter::Scatter(uint32_t pid, uint32_t flags = VMMDLL_FLAG_NOCACHE |
                                                VMMDLL_FLAG_ZEROPAD_ON_FAIL |
                                                VMMDLL_FLAG_NOPAGING)
    : pid_(pid), flags_(flags) {}

void Scatter::AddSRP(const SRP& srp) {
  srp_map_[srp.address] = srp;
  auto& entry = srp_map_[srp.address];
  entry.buffer.resize(entry.length);
  entry.scatter.qwA = entry.address;
  entry.scatter.cb = entry.length;
  entry.scatter.pb = entry.buffer.data();
}

void Scatter::AddSRP(const std::vector<SRP>& srps) {
  for (auto& srp : srps) {
    AddSRP(srp);
  }
}

void Scatter::AddSRP(uint64_t address, uint32_t length) {
  SRP srp = {.address = address, .length = length};
  AddSRP(std::move(srp));
}

bool Scatter::RemoveSRP(const SRP& srp) { return RemoveSRP(srp.address); }

bool Scatter::RemoveSRP(uint64_t address) {
  auto it = srp_map_.find(address);
  if (it == srp_map_.end()) {
    return false;
  }
  srp_map_.erase(it);
  return true;
}

SRP* Scatter::GetSRP(uint64_t address) {
  auto it = srp_map_.find(address);
  if (it == srp_map_.end()) {
    return nullptr;
  }
  return &(it->second);
}

std::vector<uint8_t>& Scatter::GetData(uint64_t address) {
  auto it = srp_map_.find(address);
  if (it == srp_map_.end()) {
    throw std::runtime_error("SRP not exist.");
  }
  return it->second.buffer;
}

std::map<uint64_t, SRP>& Scatter::SRPMap() noexcept { return srp_map_; }

const uint32_t Scatter::pid() const { return pid_; }

const uint32_t Scatter::flags() const { return flags_; }

void Scatter::SetPID(uint32_t pid) { pid_ = pid; }

void Scatter::SetFlags(uint32_t flags) { flags_ = flags; }

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
  std::vector<PMEM_SCATTER> scatter_list(scatter.SRPMap().size());
  for (auto [addr, srp] : scatter.SRPMap()) {
    scatter_list.push_back(&srp.scatter);
  }
  auto success_entries = dma_exec_->VMMCall(
      VMM::MemReadScatter, scatter.pid(), scatter_list.data(),
      scatter_list.size(), scatter.flags());
  return success_entries;
}

std::future<uint32_t> DMAIO::WriteScatter(Scatter& scatter) {
  std::vector<PMEM_SCATTER> scatter_list(scatter.SRPMap().size());
  for (auto [addr, srp] : scatter.SRPMap()) {
    scatter_list.push_back(&srp.scatter);
  }
  auto success_entries =
      dma_exec_->VMMCall(VMM::MemWriteScatter, scatter.pid(),
                         scatter_list.data(), scatter_list.size());
  return success_entries;
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
