#ifndef NETWORKPROTOCOLDSL_UV_LIBUVSERVERWRAPPER_HPP
#define NETWORKPROTOCOLDSL_UV_LIBUVSERVERWRAPPER_HPP

#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/interpretercollectionmanager.hpp>
#include <networkprotocoldsl/interpreterrunner.hpp>
#include <networkprotocoldsl_uv/asyncworkqueue.hpp>
#include <networkprotocoldsl_uv/libuvserverrunner.hpp>

#include <functional>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

// For brevity, assume networkprotocoldsl::Value is defined.
namespace networkprotocoldsl_uv {

class LibuvServerWrapper {
public:
  // Now receives program, callbacks, and an async work queue reference.
  LibuvServerWrapper(
      const networkprotocoldsl::InterpretedProgram &program,
      const networkprotocoldsl::InterpreterRunner::callback_map &callbacks,
      AsyncWorkQueue &async_queue);
  ~LibuvServerWrapper();

  // start now receives the ip and port to bind on and returns the bind result
  // future.
  std::future<LibuvServerRunner::BindResult> start(const std::string &ip,
                                                   int port);

  // Stop the server and join threads.
  void stop();

private:
  networkprotocoldsl::InterpreterCollectionManager mgr_;
  networkprotocoldsl::InterpreterRunner runner_;
  std::unique_ptr<LibuvServerRunner> uv_server_runner_;
  AsyncWorkQueue *async_queue_; // Changed: no longer owned internally.
  uv_loop_t *loop_;

  // Save the program for use in start().
  networkprotocoldsl::InterpretedProgram program_;

  std::thread interpreter_thread_;
  std::thread callback_thread_;

  // Disable copy.
  LibuvServerWrapper(const LibuvServerWrapper &) = delete;
  LibuvServerWrapper &operator=(const LibuvServerWrapper &) = delete;
};

} // namespace networkprotocoldsl_uv

#endif // NETWORKPROTOCOLDSL_UV_LIBUVSERVERWRAPPER_HPP
