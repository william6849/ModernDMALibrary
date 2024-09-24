#include <future>

template <typename T>
HandleWrapper<T>::HandleWrapper(T* handle)
    : handle_(handle, nullptr) {}

template <typename T>
HandleWrapper<T>::HandleWrapper(T* handle,
                                std::function<void(T*)> deleter)
    :handle_(handle, deleter) {}

template <typename T>
template <typename Func, typename... Args>
auto HandleWrapper<T>::Call(Func&& func, Args&&... args) {
  return func(handle_.get(), std::forward<Args>(args)...);
}

template <typename T>
template <class Rep, class Period, typename Func, typename... Args>
auto HandleWrapper<T>::Call(const std::chrono::duration<Rep, Period>& timeout,
                            Func&& func, Args&&... args) {
  auto asyncer = std::async(std::launch::async, [&]() {
    return func(handle_.get(), std::forward<Args>(args)...);
  });

  if (asyncer.wait_for(timeout) == std::future_status::timeout) {
    throw std::runtime_error("Async call timeout.");
  }

  return asyncer.get();
}

template <typename T>
void HandleWrapper<T>::reset(T* handle) {
  handle_.reset(handle);
}

template <typename T>
T* HandleWrapper<T>::get() const {
  return handle_.get();
}

template <typename T>
HandleWrapper<T>::operator T*() const {
  return handle_.get();
}
