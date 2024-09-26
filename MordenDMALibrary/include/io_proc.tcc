template <typename Func, typename... Args>
auto DMATaskExecutor::VMMCall(Func&& func, Args&&... args) {
  return Call(255, std::forward<Func>(func), vmm_handle_->get(),
              std::forward<Args>(args)...);
};

template <typename Func, typename... Args>
auto DMATaskExecutor::VMMCall(uint8_t priority, Func&& func, Args&&... args) {
  return Call(priority, std::forward<Func>(func), vmm_handle_->get(),
              std::forward<Args>(args)...);
};

template <typename Func, typename... Args>
auto DMATaskExecutor::LCCall(Func&& func, Args&&... args) {
  return Call(255, std::forward<Func>(func), lc_handle_->get(),
              std::forward<Args>(args)...);
}

template <typename Func, typename... Args>
auto DMATaskExecutor::LCCall(uint8_t priority, Func&& func, Args&&... args) {
  return Call(priority, std::forward<Func>(func), lc_handle_->get(),
              std::forward<Args>(args)...);
}

template <typename Func, typename... Args>
auto DMATaskExecutor::Call(uint8_t priority, Func&& func, Args&&... args) {
  auto pack = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
  using return_type = decltype(pack());
  std::function<return_type()> task = pack;
  return Producer(task, priority).get();
}

template <class Rep, class Period, typename Func, typename... Args>
auto DMATaskExecutor::Call(const std::chrono::duration<Rep, Period>&& timeout,
                           Func&& func, Args&&... args) {
  auto asyncer = Call(func, args...);

  if (asyncer.wait_for(timeout) == std::future_status::timeout) {
    throw std::runtime_error("Async call timeout.");
  }

  return asyncer.get();
}

template <typename Func>
auto DMATaskExecutor::Producer(Func&& task, uint8_t priority)
    -> std::future<typename std::invoke_result<Func>::type> {
  using result_type = typename std::invoke_result<Func>::type;
  auto result_promise = std::make_shared<std::promise<result_type>>();
  auto result_future = result_promise->get_future();

  auto packed_task = [task, result_promise]() {
    try {
      auto ret = task();
      result_promise->set_value(ret);
    } catch (...) {
    }
  };
  {
    std::lock_guard<std::mutex> lock(queue_mutex);
    dmatask_queue.push(DMATask{packed_task, priority});
  }
  queue_cv.notify_one();

  return result_future;
};
