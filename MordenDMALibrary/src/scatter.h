#pragma once

#include <chrono>
#include <vector>
#include <map>

struct ScatterRequestPackage {
  MEM_SCATTER scatter{
      .version = MEM_SCATTER_VERSION, .f = false, .cb = DEFAULT_PAGE_BYTES};
  uint64_t address;
  uint32_t length = DEFAULT_PAGE_BYTES;
  std::vector<uint8_t> buffer = std::vector<uint8_t>(DEFAULT_PAGE_BYTES);
};
using SRP = ScatterRequestPackage;

class Scatter {
 public:
  Scatter(uint32_t pid, uint32_t flags);

  void AddSRP(const SRP& srp);
  void AddSRP(const std::vector<SRP>& srps);
  void AddSRP(uint64_t address, uint32_t length);
  bool RemoveSRP(const SRP& srp);
  bool RemoveSRP(uint64_t address);
  SRP* GetSRP(uint64_t address);
  std::vector<uint8_t>& GetData(uint64_t address);

  std::map<uint64_t, SRP>& SRPMap() noexcept;

  const uint32_t pid() const;
  const uint32_t flags() const;
  void SetPID(uint32_t pid);
  void SetFlags(uint32_t flags);

 private:
  std::map<uint64_t, SRP> srp_map_;
  uint32_t pid_;
  uint32_t flags_;
  std::chrono::system_clock::time_point last_execution_;
};