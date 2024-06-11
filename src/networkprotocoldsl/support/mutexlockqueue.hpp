#ifndef INCLUDED_NETWORKPROTOCOLDSL_SUPPORT_MUTEXLOCKQUEUE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_SUPPORT_MUTEXLOCKQUEUE_HPP

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>

namespace networkprotocoldsl::support {

template <typename T> class MutexLockQueue {
  std::mutex mtx;
  std::deque<T> queue;

public:
  MutexLockQueue() = default;
  MutexLockQueue(const MutexLockQueue &in) = delete;
  MutexLockQueue(MutexLockQueue &&in) = delete;
  MutexLockQueue &operator=(const MutexLockQueue &) = delete;

  void push_back(const T &input) {
    std::lock_guard<std::mutex> lock(mtx);
    queue.push_back(input);
  }
  void push_back(T &&input) {
    std::lock_guard<std::mutex> lock(mtx);
    queue.push_back(std::move(input));
  }
  std::optional<T> pop() {
    std::lock_guard<std::mutex> lock(mtx);
    if (queue.empty()) {
      return std::nullopt;
    } else {
      T out = std::move(queue.front());
      queue.pop_front();
      return out;
    }
  }
  void push_front(const T &input) {
    std::lock_guard<std::mutex> lock(mtx);
    queue.push_front(input);
  }
};

} // namespace networkprotocoldsl::support

#endif