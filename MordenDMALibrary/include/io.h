#ifndef IO_H
#define IO_H

#include <chrono>
#include <future>

#include "leech_wrapper.h"

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
