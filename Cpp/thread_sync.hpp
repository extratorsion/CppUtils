#ifndef MUTEX_HPP
#define MUTEX_HPP
#include <memory>
#include <mutex>
#include <thread>

namespace {
template <typename T>
struct MutexUnit {
  template <typename... Args>
  explicit MutexUnit(Args&&... args) : data_(std::forward<Args>(args)...), mutex_() {}

  explicit MutexUnit(const T& data) : data_(data), mutex_() {}
  T data_;
  std::mutex mutex_;
};
}  // namespace

namespace sauron::utils {

template <typename T>
class SharedMutex;

template <typename T>
class MutexGuard {
 public:
  explicit MutexGuard(MutexGuard&& rhs) : data_(rhs.data_), data_lock_(rhs.data_lock_) {
    rhs.data_ = nullptr;
    rhs.data_lock_ = nullptr;
  }

  explicit MutexGuard(const MutexGuard&) = delete;

  ~MutexGuard() {
    if (data_lock_) data_lock_->unlock();
  }

  MutexGuard<T> operator=(const MutexGuard<T>&) = delete;

  MutexGuard<T> operator=(MutexGuard<T>&&) = delete;

  T& get_mut() { return *data_; }

  const T& get() const { return *data_; }

  bool valid() const { return data_lock_ && data_; }

 private:
  friend class SharedMutex<T>;
  MutexGuard(T* data, std::mutex* lock) : data_(data), data_lock_(lock) {}

 private:
  T* data_;
  std::mutex* data_lock_;
};

template <typename T>
class SharedMutex : private std::shared_ptr<MutexUnit<T>> {
 private:
  using super = std::shared_ptr<MutexUnit<T>>;

  explicit SharedMutex(MutexUnit<T>* pointer) : super(pointer) {}

 public:
  template <typename... Args>
  static inline SharedMutex<T> Create(Args&&... args) {
    return SharedMutex<T>(new MutexUnit<T>(std::forward<Args>(args)...));
  }

 public:
  explicit SharedMutex(T&& data) : super(new MutexUnit<T>(std::forward<T>(data))) {}

  explicit SharedMutex(const T& data) : super(new MutexUnit<T>(data)) {}

  SharedMutex() : super(new MutexUnit<T>(T())) {}

  MutexGuard<T> try_lock() {
    if (super::get()->mutex_.try_lock()) {
      return MutexGuard<T>(&super::get()->data_, &super::get()->mutex_);
    } else {
      return MutexGuard<T>(static_cast<T*>(nullptr), static_cast<std::mutex*>(nullptr));
    }
  }

  MutexGuard<T> lock() {
    super::get()->mutex_.lock();
    return MutexGuard<T>(&super::get()->data_, &super::get()->mutex_);
  }
};

template <typename T>
class Mutex {
 public:
  explicit Mutex(T&& data) : unit_(std::forward<T>(data)) {}
  explicit Mutex(T& data) : unit_(std::forward<T>(data)) {}

  MutexGuard<T> lock() {
    unit_.mutex_.lock();
    return MutexGuard<T>(&unit_.data_, &unit_.mutex_);
  }

  MutexGuard<T> try_lock() {
    if (unit_.mutex_.try_lock()) {
      return MutexGuard<T>(&unit_.data_, &unit_.mutex_);
    } else {
      return MutexGuard<T>(static_cast<T*>(nullptr), static_cast<std::mutex*>(nullptr));
    }
  }

 private:
  MutexUnit<T> unit_;
};

}  // namespace sauron::utils

#endif // MUTEX_HPP

