#ifndef NETWORKPROTOCOLDSL_UV_LIBUVSERVERRUNNER_HPP
#define NETWORKPROTOCOLDSL_UV_LIBUVSERVERRUNNER_HPP

#include <networkprotocoldsl/interpretercollectionmanager.hpp>

#include <networkprotocoldsl_uv/asyncworkqueue.hpp>

#include <uv.h>

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace networkprotocoldsl_uv {

/**
 * @brief Information about a successful bind operation.
 */
struct BindInfo {
  int port; ///< The port number the server is bound to.
  int fd;   ///< The file descriptor of the server socket.
};

struct LibuvServerRunnerImpl;
class LibuvServerRunner {
public:
  /**
   * @brief Result of a bind attempt.
   *
   * If successful, a BindInfo struct is returned containing the port and fd.
   * If unsuccessful, an error message is returned.
   */
  using BindResult = std::variant<BindInfo, std::string>;

  /**
   * @brief Bind to an IP address and port, and start accepting connections.
   *
   * @param mgr The InterpreterCollectionManager to use.
   * @param loop The libuv loop to use.
   * @param ip The IP address to bind to.
   * @param port The port to bind to.
   * @param program The program to use for interpreting incoming data.
   * @param async_queue The async work queue to use.
   *
   * The constructor will start accepting connections immediately.
   *
   * The BindResult will be set when the bind operation completes.
   */
  LibuvServerRunner(networkprotocoldsl::InterpreterCollectionManager &mgr,
                    uv_loop_t *loop, const std::string &ip, int port,
                    const networkprotocoldsl::InterpretedProgram &program,
                    AsyncWorkQueue &async_queue);

  /**
   * @brief Destructor.
   */
  ~LibuvServerRunner();

  /**
   * @brief Stop accepting connections and close the server.
   *
   * This method will block until the server is closed.
   */
  void stop_accepting();

  /**
   * @brief result of the bind
   *
   * This promise will be set when the bind operation completes.
   */
  std::promise<BindResult> bind_result;

  /**
   * @brief whether the server has completely stopped
   */
  std::future<void> server_stopped;

  // implementation details
  LibuvServerRunnerImpl *impl_;

  // disallow copy and assignment
  LibuvServerRunner(const LibuvServerRunner &) = delete;
  LibuvServerRunner &operator=(const LibuvServerRunner &) = delete;
};

} // namespace networkprotocoldsl_uv

#endif // NETWORKPROTOCOLDSL_UV_LIBUVSERVERRUNNER_HPP
