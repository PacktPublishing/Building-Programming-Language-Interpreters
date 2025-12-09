#include "libuvclientwrapper.hpp"
#include "asyncworkqueue.hpp"
#include "libuvclientrunner.hpp"
#include <cassert>
#include <thread>
#include <uv.h>

namespace networkprotocoldsl_uv {

LibuvClientWrapper::LibuvClientWrapper(
    const networkprotocoldsl::InterpretedProgram &program,
    const networkprotocoldsl::InterpreterRunner::callback_map &callbacks,
    AsyncWorkQueue &async_queue)
    : runner_{networkprotocoldsl::InterpreterRunner{callbacks, false}},
      async_queue_{&async_queue}, loop_{uv_default_loop()}, program_{program} {
  // ...existing initialization...
}

LibuvClientWrapper::~LibuvClientWrapper() {
  // Signal threads to exit before joining.
  runner_.exit_when_done.store(true);
  auto collection = mgr_.get_collection();
  collection->signals->wake_up_for_output.notify();
  collection->signals->wake_up_for_input.notify();
  collection->signals->wake_up_for_callback.notify();
  collection->signals->wake_up_interpreter.notify();

  if (interpreter_thread_.joinable())
    interpreter_thread_.join();
  if (callback_thread_.joinable())
    callback_thread_.join();
}

std::future<LibuvClientRunner::ConnectionResult>
LibuvClientWrapper::start(const std::string &ip, int port) {
  uv_client_runner_ = std::make_unique<LibuvClientRunner>(
      mgr_, loop_, ip, port, program_, *async_queue_);
  interpreter_thread_ =
      std::thread([this]() { runner_.interpreter_loop(mgr_); });
  callback_thread_ = std::thread([this]() { runner_.callback_loop(mgr_); });
  return uv_client_runner_->connection_result.get_future();
}

std::future<networkprotocoldsl::Value> &LibuvClientWrapper::result() {
  runner_.exit_when_done.store(true);
  mgr_.get_collection()->signals->wake_up_for_output.notify();
  mgr_.get_collection()->signals->wake_up_for_input.notify();
  mgr_.get_collection()->signals->wake_up_for_callback.notify();
  mgr_.get_collection()->signals->wake_up_interpreter.notify();
  return uv_client_runner_->client_result;
}

} // namespace networkprotocoldsl_uv
