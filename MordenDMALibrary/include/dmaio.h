#pragma once

#include <cstdint>
#include <future>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include "io_proc.h"
#include "wrappers/leech.h"
#include "wrappers/option.h"

class DMAIO {
 public:
  DMAIO();
  DMAIO(const std::string& params);
  void Reset(const std::string& params);

  std::future<std::optional<std::vector<uint8_t>>> Read(
      uint32_t pid, uint64_t virtual_addr, std::size_t bytes) const;
  std::future<bool> Write(int32_t pid, uint64_t virtual_addr,
                          std::vector<uint8_t>& data) const;
  std::future<uint32_t> ReadScatter(Scatter& scatter);
  std::future<uint32_t> WriteScatter(Scatter& scatter);

  const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> vmm_handle() const;

  const std::shared_ptr<DMATaskExecutor> GetExecutor();

 private:
  void Init(const std::string& params);
  std::shared_ptr<DMATaskExecutor> dma_exec_;
};
