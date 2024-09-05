#ifndef LEECH_WRAPPER_H
#define LEECH_WRAPPER_H

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
namespace LC {};
namespace VMM {

VMM_HANDLE Initialize(const std::string& arguments);
std::vector<uint8_t> MemReadEx(const VMM_HANDLE handle, const uint32_t pid,
                               const uint64_t addr, const size_t bytes,
                               uint32_t flag);
bool MemWrite(const VMM_HANDLE handle, const uint32_t pid, const uint64_t addr,
              std::vector<uint8_t>& data);

};  // namespace VMM

#endif
