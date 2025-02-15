#include <networkprotocoldsl_uv/asyncworkqueue.hpp>
#include <networkprotocoldsl_uv/libuvserverrunner.hpp>
#include <networkprotocoldsl_uv/libuvserverwrapper.hpp>

#include <cassert>
#include <uv.h>

namespace networkprotocoldsl_uv {

LibuvServerWrapper::LibuvServerWrapper(
    const networkprotocoldsl::InterpretedProgram &program,
    const networkprotocoldsl::InterpreterRunner::callback_map &callbacks,
    AsyncWorkQueue &async_queue)
    : runner_{networkprotocoldsl::InterpreterRunner{callbacks, false}},
      loop_{uv_default_loop()}, program_{program}, async_queue_{&async_queue}
// use the passed async queue.
{
  // Note: No creation of async work queue here.
}

LibuvServerWrapper::~LibuvServerWrapper() {
  stop();
  // ...existing cleanup...
}

std::future<LibuvServerRunner::BindResult>
LibuvServerWrapper::start(const std::string &ip, int port) {
  // Create the libuv server runner (bind happens asynchronously).
  uv_server_runner_ = std::make_unique<LibuvServerRunner>(
      mgr_, loop_, ip, port, program_, *async_queue_);
  // Launch interpreter loop thread.
  interpreter_thread_ =
      std::thread([this]() { runner_.interpreter_loop(mgr_); });
  // Launch callback loop thread.
  callback_thread_ = std::thread([this]() { runner_.callback_loop(mgr_); });
  // Return the future associated with the bind result.
  return uv_server_runner_->bind_result.get_future();
}

void LibuvServerWrapper::stop() {
  if (uv_server_runner_) {
    uv_server_runner_->stop_accepting();
  }
  // Signal the interpreter to exit.
  runner_.exit_when_done.store(true);
  mgr_.get_collection()->signals->wake_up_for_output.notify();
  mgr_.get_collection()->signals->wake_up_for_input.notify();
  mgr_.get_collection()->signals->wake_up_for_callback.notify();
  mgr_.get_collection()->signals->wake_up_interpreter.notify();
  // Join threads.
  if (interpreter_thread_.joinable())
    interpreter_thread_.join();
  if (callback_thread_.joinable())
    callback_thread_.join();
  uv_server_runner_->server_stopped.wait();
}

} // namespace networkprotocoldsl_uv
