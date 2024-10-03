#ifndef LEECH_WRAPPER_H
#define LEECH_WRAPPER_H

#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <vector>

#include "vmmdll.h"

template <typename T>
class HandleWrapper {
 public:
  HandleWrapper(T* handle) : handle_(handle, nullptr) {}
  HandleWrapper(T* handle, std::function<void(T*)> deleter)
      : handle_(handle, deleter) {}

  void reset(T* handle) { handle_.reset(handle); }

  T* get() const { return handle_.get(); }

  operator T*() const { return handle_.get(); }

 private:
  std::unique_ptr<T, std::function<void(T*)>> handle_;
};

namespace LC {
void HandleDeleter(HANDLE handle);
};
namespace VMM {

const int32_t DEFAULT_PAGE_BYTES = 4096;

void HandleDeleter(VMM_HANDLE handle);

VMM_HANDLE Initialize(const std::string& arguments);
std::optional<std::vector<uint8_t>> MemReadEx(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
    const uint32_t pid, const uint64_t addr, const std::size_t bytes,
    const uint32_t flag);
bool MemWrite(const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
              const uint32_t pid, const uint64_t addr,
              std::vector<uint8_t>& data);

bool ConfigGet(const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
               uint64_t, uint64_t&);
bool ConfigSet(const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> handle,
               uint64_t, uint64_t);

uint32_t MemReadScatter(const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> hVMM,
                        const uint32_t dwPID, PPMEM_SCATTER ppMEMs,
                        int32_t cpMEMs, int32_t flags);
uint32_t MemWriteScatter(
    const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> hVMM,
    const uint32_t dwPID, PPMEM_SCATTER ppMEMs, int32_t cpMEMs);

struct ScatterRequestPackage {
  MEM_SCATTER scatter{
      .version = MEM_SCATTER_VERSION, .f = false, .cb = DEFAULT_PAGE_BYTES};
  int64_t address;
  int32_t length = DEFAULT_PAGE_BYTES;
  std::vector<uint8_t> buffer;
};
using SRP = ScatterRequestPackage;

class Scatter {
 public:
  Scatter(uint32_t pid, uint32_t flags);

  void AddSRP(const SRP& srp);
  void AddSRP(const std::vector<SRP>& srps);
  void AddSRP(int64_t address, int32_t length);
  bool RemoveSRP(const SRP& srp);
  bool RemoveSRP(int64_t address);
  SRP* GetSRP(int64_t address);
  std::vector<uint8_t>& GetData(int64_t address);

  const std::map<int64_t, SRP>& SRPMap() noexcept;

  bool ExecuteRead();
  bool ExecuteWrite();

  void SetPID(uint32_t pid);
  void SetFlags(uint32_t flags);

 private:
  std::map<int64_t, SRP> srp_map_;
  uint32_t pid_;
  uint32_t flags_;
  std::chrono::system_clock::time_point last_execution_;
};
};  // namespace VMM

#endif
