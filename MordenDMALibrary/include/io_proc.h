#ifndef IO_PROC_H
#define IO_PROC_H

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>

#include "leech_wrapper.h"

struct DMATask {
  std::function<void()> func;
  int priority;
  bool operator<(const DMATask& other) const;
};

class DMATaskExecutor {
 public:
  DMATaskExecutor();
  ~DMATaskExecutor();

  void SetIOHandler(VMM_HANDLE handle);
  void SetIOHandler(HANDLE handle);

  operator VMM_HANDLE() const;

  const std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> vmm_handle() const;

  template <typename Func, typename... Args>
  auto VMMCall(Func&& func, Args&&... args) {
    return VMMCall(DEFAULT_TASK_PRIORITY, std::forward<Func>(func),
                   std::forward<Args>(args)...);
  };

  template <typename Func, typename... Args>
  auto VMMCall(uint8_t priority, Func&& func, Args&&... args) {
    auto future = Call(priority, std::forward<Func>(func), vmm_handle_->get(),
                       std::forward<Args>(args)...);
    return future;
  };

  template <typename Func, typename... Args>
  auto LCCall(Func&& func, Args&&... args) {
    return LCCall(DEFAULT_TASK_PRIORITY, std::forward<Func>(func),
                  std::forward<Args>(args)...);
  }

  template <typename Func, typename... Args>
  auto LCCall(uint8_t priority, Func&& func, Args&&... args) {
    auto future = Call(priority, std::forward<Func>(func), lc_handle_->get(),
                       std::forward<Args>(args)...);
    return future;
  }

 private:
  template <typename Func, typename... Args>
  auto Call(uint8_t priority, Func&& func, Args&&... args);

  void TaskConsumer();

  template <typename Func>
  auto TaskProducer(Func&& task, uint8_t priority);

  static const uint8_t DEFAULT_TASK_PRIORITY = 255;

  std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> vmm_handle_;
  std::shared_ptr<HandleWrapper<void>> lc_handle_;
  std::priority_queue<DMATask> dmatask_queue;
  std::mutex queue_mutex;
  std::condition_variable queue_cv;
  std::thread work_thread;
  std::atomic<bool> stopped;
};

template <typename Func, typename... Args>
auto DMATaskExecutor::Call(uint8_t priority, Func&& func, Args&&... args) {
  auto pack = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
  using return_type = decltype(pack());
  std::function<return_type()> task = pack;
  return TaskProducer(task, priority);
}

template <typename Func>
auto DMATaskExecutor::TaskProducer(Func&& task, uint8_t priority) {
  auto result_promise =
      std::make_shared<std::promise<std::invoke_result_t<Func>>>();
  auto result_future = result_promise->get_future();

  auto packed_task = [task, result_promise]() {
    try {
      auto ret = task();
      result_promise->set_value(ret);
    } catch (...) {
      try {
        result_promise->set_exception(std::current_exception());
      } catch (...) {
      }
    }
  };

  {
    std::lock_guard<std::mutex> lock(queue_mutex);
    dmatask_queue.push(DMATask{packed_task, priority});
  }
  queue_cv.notify_one();

  return result_future;
};

#endif
