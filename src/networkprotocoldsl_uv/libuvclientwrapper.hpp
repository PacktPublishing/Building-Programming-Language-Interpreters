#ifndef NETWORKPROTOCOLDSL_UV_LIBUVCLIENTWRAPPER_HPP
#define NETWORKPROTOCOLDSL_UV_LIBUVCLIENTWRAPPER_HPP

#include <future>
#include <memory>
#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/interpretercollectionmanager.hpp>
#include <networkprotocoldsl/interpreterrunner.hpp>
#include <networkprotocoldsl_uv/asyncworkqueue.hpp>
#include <networkprotocoldsl_uv/libuvclientrunner.hpp>
#include <string>
#include <thread>

namespace networkprotocoldsl_uv {

class LibuvClientWrapper {
public:
  LibuvClientWrapper(
      const networkprotocoldsl::InterpretedProgram &program,
      const networkprotocoldsl::InterpreterRunner::callback_map &callbacks,
      AsyncWorkQueue &async_queue);
  ~LibuvClientWrapper();

  // Start connecting; returns a future holding the connection result.
  std::future<LibuvClientRunner::ConnectionResult> start(const std::string &ip,
                                                         int port);

  // Returns a future for the interpreter's result.
  std::future<networkprotocoldsl::Value> &result();

  // ...no stop() needed as completion is indicated by result future.

private:
  networkprotocoldsl::InterpreterCollectionManager mgr_;
  networkprotocoldsl::InterpreterRunner runner_;
  std::unique_ptr<LibuvClientRunner> uv_client_runner_;
  AsyncWorkQueue *async_queue_;
  uv_loop_t *loop_;
  networkprotocoldsl::InterpretedProgram program_;

  std::thread interpreter_thread_;
  std::thread callback_thread_;

  // Disable copy.
  LibuvClientWrapper(const LibuvClientWrapper &) = delete;
  LibuvClientWrapper &operator=(const LibuvClientWrapper &) = delete;
};

} // namespace networkprotocoldsl_uv

#endif // NETWORKPROTOCOLDSL_UV_LIBUVCLIENTWRAPPER_HPP
