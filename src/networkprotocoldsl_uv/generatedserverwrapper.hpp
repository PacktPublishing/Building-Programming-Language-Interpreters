#ifndef NETWORKPROTOCOLDSL_UV_GENERATEDSERVERWRAPPER_HPP
#define NETWORKPROTOCOLDSL_UV_GENERATEDSERVERWRAPPER_HPP

#include <networkprotocoldsl_uv/asyncworkqueue.hpp>

#include <uv.h>

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace networkprotocoldsl_uv {

/**
 * @brief Result of a bind attempt.
 *
 * If successful, the file descriptor is returned.
 * If unsuccessful, an error message is returned.
 */
using BindResult = std::variant<int, std::string>;

/**
 * @brief Interface for a connection runner created by the server.
 *
 * This interface abstracts the generated ServerRunner so that
 * GeneratedServerWrapperBase can work with any protocol.
 * Each instance represents one connection's state machine.
 */
class IConnectionRunner {
public:
  virtual ~IConnectionRunner() = default;

  /**
   * @brief Start the connection (e.g., send initial greeting).
   */
  virtual void start() = 0;

  /**
   * @brief Process received bytes.
   * @param data The received data.
   * @return Number of bytes consumed.
   */
  virtual size_t on_bytes_received(std::string_view data) = 0;

  /**
   * @brief Check if there's pending output to send.
   */
  virtual bool has_pending_output() const = 0;

  /**
   * @brief Get the pending output data.
   */
  virtual std::string_view pending_output() const = 0;

  /**
   * @brief Mark bytes as written.
   * @param count Number of bytes written.
   */
  virtual void bytes_written(size_t count) = 0;

  /**
   * @brief Check if the connection should be closed.
   */
  virtual bool is_closed() const = 0;
};

/**
 * @brief Factory function type for creating connection runners.
 *
 * Called for each new connection. The factory should create a new Runner
 * instance that references the shared Handler.
 *
 * @return A unique pointer to the connection runner.
 */
using ConnectionRunnerFactory =
    std::function<std::unique_ptr<IConnectionRunner>()>;

/**
 * @brief Base class for the generated server wrapper.
 *
 * This class handles all the libuv mechanics and delegates
 * protocol-specific handling to IConnectionRunner instances.
 */
class GeneratedServerWrapperBase {
public:
  /**
   * @brief Construct a new server wrapper.
   *
   * @param runner_factory Factory function that creates a runner for each
   * connection. The runner references the shared handler.
   * @param async_queue The async work queue for libuv integration.
   */
  GeneratedServerWrapperBase(ConnectionRunnerFactory runner_factory,
                             AsyncWorkQueue &async_queue);

  ~GeneratedServerWrapperBase();

  /**
   * @brief Start the server on the given IP and port.
   *
   * @param ip The IP address to bind to.
   * @param port The port to bind to.
   * @return A future containing the bind result.
   */
  std::future<BindResult> start(const std::string &ip, int port);

  /**
   * @brief Stop accepting new connections and close the server.
   */
  void stop();

  // Disable copy
  GeneratedServerWrapperBase(const GeneratedServerWrapperBase &) = delete;
  GeneratedServerWrapperBase &
  operator=(const GeneratedServerWrapperBase &) = delete;

  // Forward declaration - defined in cpp
  struct Impl;

private:
  std::unique_ptr<Impl> impl_;
};

/**
 * @brief Adapter that wraps a generated ServerRunner as IConnectionRunner.
 *
 * Each adapter instance owns a Runner (which contains the per-connection
 * state machine) but references a shared Handler.
 *
 * @tparam Runner The instantiated ServerRunner<Handler> type.
 * @tparam Handler The handler type that satisfies the ServerHandlerConcept.
 */
template <typename Runner, typename Handler>
class ConnectionRunnerAdapter : public IConnectionRunner {
public:
  explicit ConnectionRunnerAdapter(const Handler& handler)
      : runner_(handler) {}

  void start() override {
    runner_.start();
  }

  size_t on_bytes_received(std::string_view data) override {
    return runner_.on_bytes_received(data);
  }

  bool has_pending_output() const override {
    return runner_.has_pending_output();
  }

  std::string_view pending_output() const override {
    return runner_.pending_output();
  }

  void bytes_written(size_t count) override { runner_.bytes_written(count); }

  bool is_closed() const override { return runner_.is_closed(); }

private:
  Runner runner_;  // Owns the per-connection state machine
};

/**
 * @brief A libuv-based server wrapper for generated protocol code.
 *
 * This template class provides libuv integration for any generated
 * ServerRunner class. It handles:
 * - Accepting TCP connections
 * - Reading data from connections
 * - Invoking the ServerRunner's on_bytes_received
 * - Writing output back to connections
 *
 * IMPORTANT: The Handler is shared across ALL connections. It must NOT
 * store per-connection state. Each connection gets its own Runner instance
 * (which owns the state machine), but they all reference the same Handler.
 *
 * LIFETIME: The Handler reference must remain valid for the lifetime of this
 * wrapper AND all active connections. The handler is captured by reference
 * in the runner factory and used by every connection. Destroying the handler
 * while connections are active results in undefined behavior.
 *
 * @tparam Runner The ServerRunner template instantiated with Handler
 *                (e.g., smtp::generated::ServerRunner<MyHandler>)
 * @tparam Handler The handler type satisfying ServerHandlerConcept.
 *
 * Usage:
 * @code
 *   #include "protocol.hpp"  // Your generated protocol
 *   #include <networkprotocoldsl_uv/generatedserverwrapper.hpp>
 *
 *   using namespace smtp::generated;
 *
 *   // Handler is shared - only store configuration, not per-connection state
 *   struct MyHandler {
 *       const Config& config;  // Shared configuration - OK
 *       // std::string client_name;  // Per-connection state - NOT OK
 *
 *       OpenOutput on_Open();
 *       AwaitServerEHLOResponseOutput on_AwaitServerEHLOResponse(
 *           const SMTPEHLOCommandData& msg);
 *       // ... other handlers
 *   };
 *
 *   Config config;
 *   MyHandler handler{config};  // Single handler instance
 *
 *   AsyncWorkQueue queue(uv_default_loop());
 *   GeneratedServerWrapper<ServerRunner<MyHandler>, MyHandler> server(
 *       handler, queue);
 *   auto bind_future = server.start("127.0.0.1", 8025);
 *   // ... run event loop ...
 * @endcode
 */
template <typename Runner, typename Handler>
class GeneratedServerWrapper : public GeneratedServerWrapperBase {
public:
  /**
   * @brief Construct a new server wrapper.
   *
   * @param handler Reference to the shared handler. Must outlive this wrapper.
   *                The handler is shared across all connections.
   * @param async_queue The async work queue for libuv integration.
   */
  GeneratedServerWrapper(const Handler& handler, AsyncWorkQueue &async_queue)
      : GeneratedServerWrapperBase(
            [&handler]() -> std::unique_ptr<IConnectionRunner> {
              return std::make_unique<
                  ConnectionRunnerAdapter<Runner, Handler>>(handler);
            },
            async_queue) {}
};

} // namespace networkprotocoldsl_uv

#endif // NETWORKPROTOCOLDSL_UV_GENERATEDSERVERWRAPPER_HPP
