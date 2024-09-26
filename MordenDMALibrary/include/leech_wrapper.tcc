
template <typename T>
HandleWrapper<T>::HandleWrapper(T* handle) : handle_(handle, nullptr) {}

template <typename T>
HandleWrapper<T>::HandleWrapper(T* handle, std::function<void(T*)> deleter)
    : handle_(handle, deleter) {}

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
