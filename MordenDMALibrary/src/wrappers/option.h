#pragma once

#include <memory>

template <typename T>
struct OptionStringifier {
  static std::string ToString(T val) {
    return std::to_string(static_cast<uint64_t>(val));
  }
};

template <>
struct OptionStringifier<VMMDLL_MEMORYMODEL_TP> {
  static std::string ToString(VMMDLL_MEMORYMODEL_TP val) {
    return std::string(VMMDLL_MEMORYMODEL_TOSTRING[val]);
  }
};

template <typename S>
class OptionProxy {
 public:
  explicit OptionProxy(const std::shared_ptr<DMATaskExecutor>& dma_exec,
                       uint64_t opt, bool read, bool write);
  OptionProxy& operator=(const uint64_t val) {
    Write(val);
    return *this;
  };
  operator S() { return Read(); };

  S Read();
  void Write(const uint64_t& val);

  std::string str() { return OptionStringifier<S>::ToString(Read()); };

 private:
  S value_{};
  uint64_t opt_ = 0;
  bool read_ = false;
  bool write_ = false;
  std::weak_ptr<DMATaskExecutor> dma_exec_;
};

namespace Target {
class Options {
 public:
  Options(const std::shared_ptr<DMATaskExecutor> dma_exec);
  OptionProxy<uint64_t> CORE_PRINTF_ENABLE;
  OptionProxy<uint64_t> CORE_VERBOSE;
  OptionProxy<uint64_t> CORE_VERBOSE_EXTRA;
  OptionProxy<uint64_t> CORE_VERBOSE_EXTRA_TLP;
  OptionProxy<uint64_t> CORE_MAX_NATIVE_ADDRESS;
  OptionProxy<uint64_t> CORE_LEECHCORE_HANDLE;
  OptionProxy<uint64_t> CORE_VMM_ID;
  OptionProxy<uint64_t> CORE_SYSTEM;
  OptionProxy<VMMDLL_MEMORYMODEL_TP> CORE_MEMORYMODEL;
  OptionProxy<uint64_t> CONFIG_IS_REFRESH_ENABLED;
  OptionProxy<uint64_t> CONFIG_TICK_PERIOD;
  OptionProxy<uint64_t> CONFIG_READCACHE_TICKS;
  OptionProxy<uint64_t> CONFIG_TLBCACHE_TICKS;
  OptionProxy<uint64_t> CONFIG_PROCCACHE_TICKS_PARTIAL;
  OptionProxy<uint64_t> CONFIG_PROCCACHE_TICKS_TOTAL;
  OptionProxy<uint64_t> CONFIG_VMM_VERSION_MAJOR;
  OptionProxy<uint64_t> CONFIG_VMM_VERSION_MINOR;
  OptionProxy<uint64_t> CONFIG_VMM_VERSION_REVISION;
  OptionProxy<uint64_t> CONFIG_STATISTICS_FUNCTIONCALL;
  OptionProxy<uint64_t> CONFIG_IS_PAGING_ENABLED;
  OptionProxy<uint64_t> CONFIG_DEBUG;
  OptionProxy<uint64_t> CONFIG_YARA_RULES;
  OptionProxy<uint64_t> WIN_VERSION_MAJOR;
  OptionProxy<uint64_t> WIN_VERSION_MINOR;
  OptionProxy<uint64_t> WIN_VERSION_BUILD;
  OptionProxy<uint64_t> WIN_SYSTEM_UNIQUE_ID;
  OptionProxy<uint64_t> FORENSIC_MODE;
  OptionProxy<uint64_t> REFRESH_ALL;
  OptionProxy<uint64_t> REFRESH_FREQ_MEM;
  OptionProxy<uint64_t> REFRESH_FREQ_MEM_PARTIAL;
  OptionProxy<uint64_t> REFRESH_FREQ_TLB;
  OptionProxy<uint64_t> REFRESH_FREQ_TLB_PARTIAL;
  OptionProxy<uint64_t> REFRESH_FREQ_FAST;
  OptionProxy<uint64_t> REFRESH_FREQ_MEDIUM;
  OptionProxy<uint64_t> REFRESH_FREQ_SLOW;
  OptionProxy<uint64_t> PROCESS_DTB;
  OptionProxy<uint64_t> PROCESS_DTB_FAST_LOWINTEGRITY;

 private:
};
}  // namespace Target