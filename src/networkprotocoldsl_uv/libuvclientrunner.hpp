#ifndef NETWORKPROTOCOLDSL_UV_LIBUVCLIENTRUNNER_HPP
#define NETWORKPROTOCOLDSL_UV_LIBUVCLIENTRUNNER_HPP

#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/interpretercollectionmanager.hpp>
#include <networkprotocoldsl/value.hpp>

#include <networkprotocoldsl_uv/asyncworkqueue.hpp>

#include <uv.h>

#include <future>
#include <string>
#include <variant>

namespace networkprotocoldsl_uv {

class LibuvClientRunner {
public:
  /**
   * @brief Result of a connection attempt.
   *
   * If successful, the file descriptor is returned, which is also the
   * key in the InterpreterCollectionManager. If unsuccessful, an error
   * message is returned.
   */
  using ConnectionResult = std::variant<int, std::string>;

  /**
   * @brief Connect to an IP address and port.
   *
   * @param mgr The InterpreterCollectionManager to use.
   * @param loop The libuv loop to use.
   * @param ip The IP address to connect to.
   * @param port The port to connect to.
   * @param program The program to use for interpreting incoming data.
   * @param async_queue The async work queue to use.
   *
   * The constructor will start connecting immediately. The ConnectionResult
   * will be set when the connection operation completes.
   *
   */
  LibuvClientRunner(networkprotocoldsl::InterpreterCollectionManager &mgr,
                    uv_loop_t *loop, const std::string &ip, int port,
                    const networkprotocoldsl::InterpretedProgram &program,
                    AsyncWorkQueue &async_queue);

  /**
   * @brief Destructor.
   */
  ~LibuvClientRunner();

  /**
   * @brief promise for the connection result.
   *
   * The promise will be set when the connection operation completes.
   */
  std::promise<ConnectionResult> connection_result;

  /**
   * @brief promise for the client result
   *
   * The promise will be set when the client interpreter returns,
   * or with an error if the connection fails.
   */
  std::promise<networkprotocoldsl::Value> client_result_promise;

  /**
   * @brief future of the value for the client
   *
   * The future will be set when the client interpreter returns,
   * or with an error if the connection fails.
   */
  std::future<networkprotocoldsl::Value> client_result;

  /**
   * @brief future from the interpreter (internal use)
   *
   * This is set when the interpreter is inserted, and will be used
   * to propagate the result to client_result_promise.
   */
  std::future<networkprotocoldsl::Value> interpreter_result_future;

  /**
   * @brief the thread which pushes data from the interpter to uv
   *
   * The thread will be created when the connection operation completes.
   * And it will be joined when the destructor is called.
   */
  std::optional<std::thread> pusher_thread;

  // disallow copy and assignment
  LibuvClientRunner(const LibuvClientRunner &) = delete;
  LibuvClientRunner &operator=(const LibuvClientRunner &) = delete;
};

} // namespace networkprotocoldsl_uv

#endif // NETWORKPROTOCOLDSL_UV_LIBUVCLIENTRUNNER_HPP
