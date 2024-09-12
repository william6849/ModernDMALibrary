#ifndef LEECH_WRAPPER_H
#define LEECH_WRAPPER_H

#include <chrono>
#include <memory>
#include <mutex>
#include <vector>

#include "vmmdll.h"

void VMMHandleDeleter(VMM_HANDLE handle);

class VMMHandleWrapper {
 public:
  VMMHandleWrapper() {};
  VMMHandleWrapper(VMM_HANDLE handle);

  VMMHandleWrapper& operator=(
      const std::shared_ptr<tdVMM_HANDLE>& other) noexcept {
    handle_ = other;
    return *this;
  };

  template <typename Func, typename... Args>
  auto Call(Func&& func, Args&&... args) {
    std::lock_guard<std::mutex> lock(*mutex_);
    return func(handle_.get(), std::forward<Args>(args)...);
  }

  void reset(VMM_HANDLE handle) noexcept;

  VMM_HANDLE get() const;
  operator VMM_HANDLE() const;

 private:
  std::shared_ptr<tdVMM_HANDLE> handle_;
  std::shared_ptr<std::mutex> mutex_;
};

void HandleDeleter(HANDLE handle);

class LCHandleWrapper {
 public:
  LCHandleWrapper() {};
  LCHandleWrapper(HANDLE handle);

  LCHandleWrapper& operator=(const std::shared_ptr<void>& other) noexcept {
    handle_ = other;
    return *this;
  };

  template <typename Func, typename... Args>
  auto Call(Func&& func, Args&&... args) {
    std::lock_guard<std::mutex> lock(*mutex_);
    return func(handle_.get(), std::forward<Args>(args)...);
  }

  void reset(HANDLE handle) noexcept;

  HANDLE get() const;
  operator HANDLE() const;

 private:
  std::shared_ptr<void> handle_;
  std::shared_ptr<std::mutex> mutex_;
};

namespace LC {};
namespace VMM {

VMM_HANDLE Initialize(const std::string& arguments);
std::vector<uint8_t> MemReadEx(const VMM_HANDLE handle, const uint32_t pid,
                               const uint64_t addr, const size_t bytes,
                               uint32_t flag);
bool MemWrite(const VMM_HANDLE handle, const uint32_t pid, const uint64_t addr,
              std::vector<uint8_t>& data);

struct ScatterRequestPackage {
  uint64_t address = 0;
  uint64_t length = 0;
};
typedef ScatterRequestPackage SRP;

class Scatter {
 public:
  Scatter();
  Scatter(uint32_t pid, uint32_t flags);
  ~Scatter();

  bool AddSRP(SRP srp);
  bool AddSRP(const std::vector<SRP>& srps);
  bool RemoveSRP(SRP srp);
  const std::vector<SRP>& SRPList() noexcept;

  bool ExecuteRead();
  const std::vector<uint8_t>& Read(uint64_t address);
  const std::vector<uint8_t>& Read(uint64_t address, uint64_t length);
  bool ExecuteWrite();
  bool Write(uint64_t address, const std::vector<uint8_t>& data);

  void SetPID(uint32_t pid);
  void SetFlags(uint32_t flags);

 private:
  std::unordered_map<uint64_t, SRP> srp_list_;
  std::map<SRP, std::vector<uint8_t>> sr_buffer;
  uint32_t pid_;
  uint32_t flags_;
  std::chrono::system_clock::time_point last_execute_;
};
};  // namespace VMM

#endif
