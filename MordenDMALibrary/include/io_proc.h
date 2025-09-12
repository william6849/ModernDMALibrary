#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>

struct DMATask {
  std::move_only_function<void()> func;
  int priority;
  bool operator<(const DMATask& other) const const {
    return priority > other.priority;
  };
};

class DMATaskExecutor {
 public:
  DMATaskExecutor();
  ~DMATaskExecutor();
  DMATaskExecutor(const DMATaskExecutor& other) = delete;
  DMATaskExecutor& operator=(const DMATaskExecutor& other) = delete;
  DMATaskExecutor(DMATaskExecutor&& other) = delete;
  DMATaskExecutor& operator=(DMATaskExecutor&& other) = delete;

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
    auto future = Call(priority, std::forward<Func>(func), vmm_handle_,
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
    auto future = Call(priority, std::forward<Func>(func), lc_handle_,
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
  using return_type = std::invoke_result_t<Func, Args...>;
  auto task_wrapper = [func = std::forward<Func>(func),
                       ... args =
                           std::forward<Args>(args)]() mutable -> return_type {
    if constexpr (std::is_void_v<return_type>) {
      std::invoke(func, std::forward<Args>(args)...);
    } else {
      return std::invoke(func, std::forward<Args>(args)...);
    }
  };
  return TaskProducer(std::move(task_wrapper), priority);
}

template <typename Func>
auto DMATaskExecutor::TaskProducer(Func&& task, uint8_t priority) {
  using ReturnType = std::invoke_result_t<Func>;

  std::packaged_task<ReturnType()> task_pkg(std::forward<Func>(task));
  auto result_future = task_pkg->get_future();

  {
    std::lock_guard<std::mutex> lock(queue_mutex);
    dmatask_queue.push(
        DMATask{[task = std::move(task_pkg)]() mutable { task(); }, priority});
  }
  queue_cv.notify_one();

  return result_future;
};
