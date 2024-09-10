#ifndef LEECH_WRAPPER_H
#define LEECH_WRAPPER_H

#include <chrono>
#include <memory>
#include <vector>

#include "vmmdll.h"

struct VMMHandleDeleter {
  void operator()(VMM_HANDLE handle) const;
};

using UniqueVMMHandle = std::unique_ptr<tdVMM_HANDLE, VMMHandleDeleter>;

class VMMHandleWrapper {
 public:
  explicit VMMHandleWrapper(VMM_HANDLE handle);

  VMMHandleWrapper(const VMMHandleWrapper&) = delete;
  VMMHandleWrapper& operator=(const VMMHandleWrapper&) = delete;

  VMMHandleWrapper(VMMHandleWrapper&& other) noexcept;
  VMMHandleWrapper& operator=(VMMHandleWrapper&& other) noexcept;

  VMM_HANDLE get() const;
  operator VMM_HANDLE() const;

 private:
  UniqueVMMHandle handle_;
};

struct LCHandleDeleter {
  void operator()(HANDLE handle) const;
};

using UniqueLCHandle = std::unique_ptr<void, LCHandleDeleter>;

struct ScatterHandleDeleter {
  void operator()(HANDLE handle) const;
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
