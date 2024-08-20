#include "leechwrapper.h"

void VMMHandleDeleter::operator()(VMM_HANDLE handle) const {
  if (handle) {
    VMMDLL_Close(handle);
  }
}

VMMHandleWrapper::VMMHandleWrapper(VMM_HANDLE handle) : handle_(handle) {}

VMMHandleWrapper::VMMHandleWrapper(VMMHandleWrapper&& other) noexcept
    : handle_(std::move(other.handle_)) {}

VMMHandleWrapper& VMMHandleWrapper::operator=(
    VMMHandleWrapper&& other) noexcept {
  if (this != &other) {
    handle_ = std::move(other.handle_);
    other.handle_ = nullptr;
  }
  return *this;
}

VMM_HANDLE VMMHandleWrapper::get() const { return handle_.get(); }

VMMHandleWrapper::operator VMM_HANDLE() const { return handle_.get(); }
