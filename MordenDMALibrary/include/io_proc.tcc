template <typename Func, typename... Args>
auto DMATaskExecutor::VMMCall(Func&& func, Args&&... args) {
  return VMMCall(std::forward<uint8_t>(255), std::forward<Func>(func),
                 std::forward<Args>(args)...);
};

template <typename Func, typename... Args>
auto DMATaskExecutor::VMMCall(uint8_t priority, Func&& func, Args&&... args) {
  auto future = Call(priority, std::forward<Func>(func), vmm_handle_->get(),
                     std::forward<Args>(args)...);
  return future;
};

template <typename Func, typename... Args>
auto DMATaskExecutor::LCCall(Func&& func, Args&&... args) {
  return LCCall(std::forward<uint8_t>(255), std::forward<Func>(func),
                std::forward<Args>(args)...);
}

template <typename Func, typename... Args>
auto DMATaskExecutor::LCCall(uint8_t priority, Func&& func, Args&&... args) {
  auto future = Call(priority, std::forward<Func>(func), lc_handle_->get(),
                     std::forward<Args>(args)...);
  return future;
}

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
