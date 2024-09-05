#ifndef IO_H
#define IO_H

#include <cstdint>
#include <string>
#include <vector>

#include "leech_wrapper.h"

template <typename S>
class OptionProxy {
 public:
  explicit OptionProxy(uint64_t opt, bool read, bool write);
  OptionProxy& operator=(const uint64_t& val) {
    Write(val);
    return *this;
  };
  operator S() { return Read(); };

  virtual S Read();
  virtual void Write(const uint64_t& val);

  template <typename U = S>
  typename std::enable_if<std::is_same<U, VMMDLL_MEMORYMODEL_TP>::value,
                          std::string>::type
  str() {
    return std::string(VMMDLL_MEMORYMODEL_TOSTRING[Read()]);
  }

 private:
  S value_ = static_cast<S>(0);
  uint64_t opt_ = 0;
  bool read_ = false;
  bool write_ = false;
};

namespace Target {
class Options {
 public:
  Options();
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
  Options(const Options&) = delete;
  Options& operator=(const Options&) = delete;
  Options(Options&&) = delete;
  Options& operator=(Options&&) = delete;
};

}  // namespace Target

class DMAIO {
 public:
  DMAIO();
  DMAIO(const std::string& params);
  void Reset(const std::string& params);

  std::vector<uint8_t> Read(uint64_t addr, size_t bytes) const;
  bool Write(uint64_t addr, std::vector<uint8_t> data) const;

  operator VMM_HANDLE() const;
  const VMM_HANDLE vmm_handle() const;

 private:
  void Init(const std::string& params);
  UniqueVMMHandle vmm_handle_;
  UniqueLCHandle lc_handle_;
};

#endif
