#ifndef LEECH_WRAPPER_H
#define LEECH_WRAPPER_H

#include <memory>

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

#endif
