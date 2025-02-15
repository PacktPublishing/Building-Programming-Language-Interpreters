#include "libuvclientrunner.hpp"
#include "asyncworkqueue.hpp"
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <networkprotocoldsl/support/mutexlockqueue.hpp>
#include <thread>
#include <uv.h>

namespace networkprotocoldsl_uv {
using namespace networkprotocoldsl;

namespace {

// Removed WriteCallbackContext.

struct LibuvContext {
  LibuvClientRunner *runner;
  networkprotocoldsl::InterpreterCollectionManager *mgr;
  InterpretedProgram program;
  uv_loop_t *loop;
  networkprotocoldsl_uv::AsyncWorkQueue *work_queue; // injected work queue.
};

struct UvConnectionData {
  LibuvContext *context;
  uv_tcp_t conn;
  int fd;
};

void alloc_buffer_cb(uv_handle_t *handle, size_t suggested, uv_buf_t *buf) {
  buf->base = static_cast<char *>(malloc(suggested));
  buf->len = suggested;
}

// New unified close callback for client connections.
static void unified_close_cb(uv_handle_t *handle) {
  UvConnectionData *data = static_cast<UvConnectionData *>(handle->data);
  delete data->context;
  delete data;
}

// Updated on_read callback to push received data into input_buffer.
void on_read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
  UvConnectionData *conn_data = static_cast<UvConnectionData *>(stream->data);
  // Retrieve the interpreter context from the collection.
  auto collection = conn_data->context->mgr->get_collection();
  auto it = collection->interpreters.find(conn_data->fd);
  if (it == collection->interpreters.end()) {
    if (buf->base)
      free(buf->base);
    // interpreter was removed, so just unregister all callbacks.
    uv_read_stop(stream);
    // Use unified_close_cb here.
    uv_close(reinterpret_cast<uv_handle_t *>(stream), unified_close_cb);
    return;
  }
  if (nread > 0) {
    std::string input_data(buf->base, nread);
    // Push data to input_buffer.
    it->second->input_buffer.push_back(input_data);
    // Notify the interpreter that input is available.
    collection->signals->wake_up_for_input.notify();
    collection->signals->wake_up_interpreter.notify();
  } else if (nread < 0) {
    // Handle error/connection close (optional: mark EOF).
    it->second->eof.store(true);
    collection->signals->wake_up_for_input.notify();
    collection->signals->wake_up_interpreter.notify();
  }
  if (buf->base)
    free(buf->base);
}

// Updated free static write callback using UvConnectionData directly.
static void on_write_completed(uv_write_t *req, int status) {
  auto *conn_data = static_cast<UvConnectionData *>(req->data);
  auto collection = conn_data->context->mgr->get_collection();
  auto it = collection->interpreters.find(conn_data->fd);
  if (it != collection->interpreters.end() && status < 0) {
    it->second->eof.store(true);
    collection->signals->wake_up_for_output.notify();
    collection->signals->wake_up_interpreter.notify();
    uv_read_stop(reinterpret_cast<uv_stream_t *>(&conn_data->conn));
    // Use unified_close_cb here.
    uv_close(reinterpret_cast<uv_handle_t *>(&conn_data->conn),
             unified_close_cb);
  }
  delete req;
}

// Updated pusher thread loop to use UvConnectionData in the write callback.
void pusher_thread_loop(LibuvClientRunner *r, UvConnectionData *data) {
  while (true) {
    bool did_something = false;
    auto collection = data->context->mgr->get_collection();
    // Exit if the interpreter was removed.
    auto it = collection->interpreters.find(data->fd);
    if (it == collection->interpreters.end() || it->second->exited.load()) {
      data->context->work_queue->push_work([data]() {
        uv_read_stop(reinterpret_cast<uv_stream_t *>(&data->conn));
        // Use unified_close_cb here.
        uv_close(reinterpret_cast<uv_handle_t *>(&data->conn),
                 unified_close_cb);
      });
      return;
    }
    // Pop outgoing data from the interpreter's output_buffer.
    auto &interpreter_context = it->second;
    auto output_opt = interpreter_context->output_buffer.pop();
    if (output_opt.has_value()) {
      did_something = true;
      std::string output = output_opt.value();
      // Enqueue a write task using UvConnectionData for the callback.
      data->context->work_queue->push_work([data, output, collection]() {
        uv_write_t *req = new uv_write_t;
        req->data = data; // set UvConnectionData as the write callback data.
        uv_buf_t wrbuf =
            uv_buf_init(const_cast<char *>(output.data()), output.size());
        uv_write(req, reinterpret_cast<uv_stream_t *>(&data->conn), &wrbuf, 1,
                 on_write_completed);
      });
    }
    // If nothing was found, wait for wake_up_for_output notification.
    if (!did_something) {
      collection->signals->wake_up_for_output.wait();
    }
  }
}

static void on_connect_client_cb(uv_connect_t *client, int status) {
  UvConnectionData *conn_data =
      reinterpret_cast<UvConnectionData *>(client->data);
  auto *runner = conn_data->context->runner;
  if (status < 0) {
    runner->connection_result.set_value(std::string(uv_strerror(status)));
    delete client;
    // Instead of manually deleting conn_data, close the handle so
    // unified_close_cb deletes it.
    uv_close(reinterpret_cast<uv_handle_t *>(&conn_data->conn),
             unified_close_cb);
    return;
  }
  uv_tcp_t *handle = reinterpret_cast<uv_tcp_t *>(client->handle);
  handle->data = conn_data;
  uv_os_fd_t fd;
  if (uv_fileno(reinterpret_cast<uv_handle_t *>(handle), &fd) == 0) {
    conn_data->fd = fd;
    runner->client_result = conn_data->context->mgr->insert_interpreter(
        static_cast<int>(fd), conn_data->context->program, std::nullopt,
        conn_data);
    runner->connection_result.set_value(static_cast<int>(fd));
    uv_read_start(reinterpret_cast<uv_stream_t *>(handle), alloc_buffer_cb,
                  on_read_cb);
    // Create pusher thread using pusher_thread_loop.
    runner->pusher_thread.emplace(
        std::thread(pusher_thread_loop, runner, conn_data));
  } else {
    runner->connection_result.set_value(
        "Connection did not provide a valid file descriptor.");
    uv_close(reinterpret_cast<uv_handle_t *>(handle), unified_close_cb);
    delete client;
    return;
  }
  delete client;
}

} // namespace

LibuvClientRunner::LibuvClientRunner(
    networkprotocoldsl::InterpreterCollectionManager &mgr, uv_loop_t *loop,
    const std::string &ip, int port, const InterpretedProgram &program,
    networkprotocoldsl_uv::AsyncWorkQueue &async_queue) {
  struct sockaddr_in connect_addr;
  uv_ip4_addr(ip.c_str(), port, &connect_addr);
  UvConnectionData *data = new UvConnectionData();
  // Create context with injected async work queue.
  auto *ctx = new LibuvContext{this, &mgr, program, loop, &async_queue};
  data->context = ctx;
  // Schedule uv_tcp_init and uv_tcp_connect from the loop thread.
  async_queue.push_work([=]() {
    uv_tcp_init(loop, &data->conn);
    uv_connect_t *connect_req = new uv_connect_t;
    connect_req->data = data;
    uv_tcp_connect(connect_req, &data->conn,
                   reinterpret_cast<const struct sockaddr *>(&connect_addr),
                   on_connect_client_cb);
  });
}

LibuvClientRunner::~LibuvClientRunner() {
  // ...existing cleanup...
  if (pusher_thread && pusher_thread->joinable()) {
    pusher_thread->join();
  }
  // ...other cleanup...
}

} // namespace networkprotocoldsl_uv
