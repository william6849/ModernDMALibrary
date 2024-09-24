#ifndef LEECH_WRAPPER_H
#define LEECH_WRAPPER_H

#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include "vmmdll.h"

template <typename T>
class HandleWrapper {
 public:
  HandleWrapper(T* handle);
  HandleWrapper(T* handle, std::function<void(T*)> deleter);

  template <typename Func, typename... Args>
  auto Call(Func&& func, Args&&... args);

  template <class Rep, class Period, typename Func, typename... Args>
  auto Call(const std::chrono::duration<Rep, Period>& timeout, Func&& func,
            Args&&... args);

  void reset(T* handle);
  T* get() const;
  operator T*() const;

 private:
  std::unique_ptr<T, std::function<void(T*)>> handle_;
};
#include "handle_wrapper.tcc"

namespace LC {
void HandleDeleter(HANDLE handle);
};
namespace VMM {
void HandleDeleter(VMM_HANDLE handle);

VMM_HANDLE Initialize(const std::string& arguments);
std::vector<uint8_t> MemReadEx(const VMM_HANDLE handle, const uint32_t pid,
                               const uint64_t addr, const size_t bytes,
                               uint32_t flag);
bool MemWrite(const VMM_HANDLE handle, const uint32_t pid, const uint64_t addr,
              std::vector<uint8_t>& data);

int32_t MemReadScatter(const VMM_HANDLE hVMM, const int32_t dwPID,
                       PPMEM_SCATTER ppMEMs, int32_t cpMEMs, int32_t flags);
int32_t MemWriteScatter(const VMM_HANDLE hVMM, int32_t dwPID,
                        PPMEM_SCATTER ppMEMs, int32_t cpMEMs);

struct ScatterRequestPackage {
  MEM_SCATTER scatter{.version = MEM_SCATTER_VERSION, .f = false, .cb = 4096};
  int64_t address;
  int32_t length = 4096;
  std::vector<uint8_t> buffer;
};
typedef ScatterRequestPackage SRP;

class Scatter {
 public:
  Scatter(const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>>& handle_wrapper,
          uint32_t pid, uint32_t flags);

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
  std::weak_ptr<HandleWrapper<tdVMM_HANDLE>> handle_;
};
};  // namespace VMM

#endif
