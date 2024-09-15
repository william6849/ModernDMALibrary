#ifndef LEECH_WRAPPER_H
#define LEECH_WRAPPER_H

#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "vmmdll.h"

template <typename T>
class HandleWrapper {
 public:
  HandleWrapper(T* handle);
  HandleWrapper(T* handle, void (*deleter)(T*));

  template <typename Func, typename... Args>
  auto Call(Func&& func, Args&&... args);

  void reset(T* handle, void (*deleter)(T*));
  T* get() const;
  operator T*() const;

 private:
  std::unique_ptr<T, void (*)(T*)> handle_;
  std::mutex mutex_;
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

struct ScatterRequestPackage {
  DWORD version = MEM_SCATTER_VERSION;
  bool f = false;
  int64_t address;
  uint8_t* pb = buffer.data();
  int64_t _filler;
  int32_t length = 4096;
  int32_t iStack;
  int64_t vStack[MEM_SCATTER_STACK_SIZE];

  std::vector<uint8_t> buffer;
};
typedef ScatterRequestPackage SRP;

class Scatter {
 public:
  Scatter(const HandleWrapper<VMM_HANDLE>& handle_wrapper);
  Scatter(uint32_t pid, uint32_t flags);

  bool AddSRP(const SRP& srp);
  bool AddSRP(const std::vector<SRP>& srps);
  bool AddSRP(int64_t address, int32_t length);
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
