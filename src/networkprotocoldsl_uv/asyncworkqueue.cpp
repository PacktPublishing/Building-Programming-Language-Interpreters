#include "asyncworkqueue.hpp"
#include <stdexcept>

namespace networkprotocoldsl_uv {

static void async_work_callback(uv_async_t *handle) {
  AsyncWorkQueue *queue = static_cast<AsyncWorkQueue *>(handle->data);
  queue->process();
}

// Callback invoked by uv_close during shutdown.
static void shutdown_close_cb(uv_handle_t *handle) {
  AsyncWorkQueue *self = static_cast<AsyncWorkQueue *>(handle->data);
  self->shutdown_promise.set_value();
}

AsyncWorkQueue::AsyncWorkQueue(uv_loop_t *loop) {
  uv_async_init(loop, &async_handle_, async_work_callback);
  async_handle_.data = this;
}

AsyncWorkQueue::~AsyncWorkQueue() {
  // If shutdown wasn't invoked already, trigger it and wait.
  if (!shutdown_called_) {
    shutdown().wait();
  }
}

void AsyncWorkQueue::push_work(std::function<void()> work) {
  if (shutdown_called_.load()) {
    throw std::runtime_error(
        "Cannot push work after shutdown has been initiated");
  }
  work_queue_.push_back(work);
  uv_async_send(&async_handle_);
}

void AsyncWorkQueue::process() {
  while (true) {
    auto work_opt = work_queue_.pop();
    if (!work_opt.has_value())
      break;
    work_opt.value()();
  }
}

uv_async_t *AsyncWorkQueue::get_async_handle() { return &async_handle_; }

std::future<void> AsyncWorkQueue::shutdown() {
  if (!shutdown_called_.load()) {
    // Submit a work item that calls uv_close with a callback that fulfills
    // shutdown_promise_.
    push_work([this]() {
      uv_close(reinterpret_cast<uv_handle_t *>(&async_handle_),
               shutdown_close_cb);
      shutdown_called_.store(true);
    });
  }
  return shutdown_promise.get_future();
}

} // namespace networkprotocoldsl_uv
