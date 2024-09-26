#ifndef IO_PROC_H
#define IO_PROC_H

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
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
  auto VMMCall(Func&& func, Args&&... args);

  template <typename Func, typename... Args>
  auto VMMCall(uint8_t priority, Func&& func, Args&&... args);

  template <typename Func, typename... Args>
  auto LCCall(Func&& func, Args&&... args);

  template <typename Func, typename... Args>
  auto LCCall(uint8_t priority, Func&& func, Args&&... args);

 private:
  template <typename Func, typename... Args>
  auto Call(uint8_t priority, Func&& func, Args&&... args);

  template <class Rep, class Period, typename Func, typename... Args>
  auto Call(const std::chrono::duration<Rep, Period>&& timeout, Func&& func,
            Args&&... args);

  void Consumer();

  template <typename Func>
  auto Producer(Func&& task, uint8_t priority)
      -> std::future<typename std::invoke_result<Func>::type>;

  std::shared_ptr<HandleWrapper<tdVMM_HANDLE>> vmm_handle_;
  std::shared_ptr<HandleWrapper<void>> lc_handle_;
  std::priority_queue<DMATask> dmatask_queue;
  std::mutex queue_mutex;
  std::condition_variable queue_cv;
  std::thread work_thread;
  bool stopped;
};
#include "io_proc.tcc"

#endif
