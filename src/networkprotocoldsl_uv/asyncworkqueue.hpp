#ifndef NETWORKPROTOCOLDSL_UV_ASYNCWORKQUEUE_HPP
#define NETWORKPROTOCOLDSL_UV_ASYNCWORKQUEUE_HPP

#include <atomic>
#include <functional>
#include <future>
#include <networkprotocoldsl/support/mutexlockqueue.hpp>
#include <uv.h>

namespace networkprotocoldsl_uv {

class AsyncWorkQueue {
public:
  AsyncWorkQueue(uv_loop_t *loop);
  ~AsyncWorkQueue();

  // Enqueue a work item and signal the loop.
  void push_work(std::function<void()> work);

  // Process all pending work items.
  void process();

  // Returns the uv_async_t handle.
  uv_async_t *get_async_handle();

  // Safe shutdown: returns a future that is fulfilled when shutdown completes.
  std::future<void> shutdown();
  std::promise<void> shutdown_promise;

private:
  uv_async_t async_handle_;
  networkprotocoldsl::support::MutexLockQueue<std::function<void()>>
      work_queue_;
  // Updated to use std::atomic_bool for safe concurrent access.
  std::atomic_bool shutdown_called_{false};
};

} // namespace networkprotocoldsl_uv

#endif // NETWORKPROTOCOLDSL_UV_ASYNCWORKQUEUE_HPP
