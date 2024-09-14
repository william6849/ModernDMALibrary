template <typename T>
HandleWrapper<T>::HandleWrapper(T* handle) {
  handle_ = std::unique_ptr<T>(handle);
}

template <typename T>
HandleWrapper<T>::HandleWrapper(T* handle, void (*deleter)(T*)) {
  handle_ = std::unique_ptr<T, void (*)(T*)>(handle, deleter);
}

template <typename T>
template <typename Func, typename... Args>
auto HandleWrapper<T>::Call(Func&& func, Args&&... args) {
  std::lock_guard<std::mutex> lock(mutex_);
  return func(handle_.get(), std::forward<Args>(args)...);
}

template <typename T>
void HandleWrapper<T>::reset(T* handle, void (*deleter)(T*)) {
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
