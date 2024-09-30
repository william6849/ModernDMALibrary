#include "io_proc.h"

bool DMATask::operator<(const DMATask& other) const {
  return priority > other.priority;
};

DMATaskExecutor::DMATaskExecutor()
    : vmm_handle_(new HandleWrapper<tdVMM_HANDLE>(nullptr, VMM::HandleDeleter)),
      lc_handle_(new HandleWrapper<void>(nullptr, LC::HandleDeleter)),
      stopped(false),
      work_thread(&DMATaskExecutor::TaskConsumer, this) {}

DMATaskExecutor::~DMATaskExecutor() {
  stopped = true;
  queue_cv.notify_all();
  work_thread.join();
}

void DMATaskExecutor::SetIOHandler(VMM_HANDLE handle) {
  vmm_handle_->reset(handle);
}
void DMATaskExecutor::SetIOHandler(HANDLE handle) { lc_handle_->reset(handle); }

DMATaskExecutor::operator VMM_HANDLE() const { return vmm_handle_->get(); }

const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> DMATaskExecutor::vmm_handle()
    const {
  return vmm_handle_;
}

void DMATaskExecutor::TaskConsumer() {
  while (true) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    queue_cv.wait(lock, [&] { return !dmatask_queue.empty() || stopped; });
    if (stopped && dmatask_queue.empty()) {
      break;
    }
    DMATask task = dmatask_queue.top();
    dmatask_queue.pop();
    task.func();
    lock.unlock();
  }
};
