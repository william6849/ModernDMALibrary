#ifndef LEECH_WRAPPER_H
#define LEECH_WRAPPER_H

#include <cstdint>
#include <cstring>
#include <expected>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_set>
#include <vector>

#include "api/*"

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
void HandleDeleter(VMM_HANDLE handle);
};  // namespace VMM

#endif

#include "leechcore.h"
#include "vmmdll.h"

#include "leechcore.h"
#include "vmmdll.h"