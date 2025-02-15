#include "libuvserverrunner.hpp"
#include "asyncworkqueue.hpp"
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>
#include <uv.h>

namespace networkprotocoldsl_uv {
using namespace networkprotocoldsl;

struct LibuvServerRunnerImpl {
  networkprotocoldsl::InterpreterCollectionManager *mgr_;
  uv_loop_t *loop_;
  uv_tcp_t server_;
  InterpretedProgram program;
  networkprotocoldsl_uv::AsyncWorkQueue *work_queue;
  std::atomic<bool> exit_when_done{false};
  std::thread pusher_thread;
  std::promise<void> server_stopped;
};

namespace {

struct UvConnectionData {
  LibuvServerRunnerImpl *runner;
  std::atomic<bool> connection_close_request_sent{false};
  uv_tcp_t conn;
  int fd;
};

static void on_close_cb(uv_handle_t *handle) {
  auto *conn_data = static_cast<UvConnectionData *>(handle->data);
  auto collection = conn_data->runner->mgr_->get_collection();
  collection->signals->wake_up_for_callback.notify();
  collection->signals->wake_up_for_input.notify();
  collection->signals->wake_up_for_output.notify();
  collection->signals->wake_up_interpreter.notify();
  delete static_cast<UvConnectionData *>(handle->data);
}

static void on_write_completed(uv_write_t *req, int status) {
  auto *conn_data = static_cast<UvConnectionData *>(req->data);
  auto collection = conn_data->runner->mgr_->get_collection();
  auto it = collection->interpreters.find(conn_data->fd);
  if (it != collection->interpreters.end() && status < 0) {
    it->second->eof.store(true);
    collection->signals->wake_up_interpreter.notify();
    uv_read_stop(reinterpret_cast<uv_stream_t *>(&conn_data->conn));
  }
  delete req;
}

static void pusher_thread_loop_all(LibuvServerRunnerImpl *impl) {
  while (true) {
    auto collection = impl->mgr_->get_collection();
    int active_interpreters = 0;
    int blocked_interpreters = 0;
    for (const auto &entry : collection->interpreters) {
      auto interpreter = entry.second;
      // Each interpreter's additional_data holds its UvConnectionData.
      UvConnectionData *conn_data =
          static_cast<UvConnectionData *>(interpreter->additional_data);
      if (interpreter->exited.load()) {
        if (!conn_data->connection_close_request_sent.load()) {
          conn_data->connection_close_request_sent.store(true);
          impl->mgr_->remove_interpreter(conn_data->fd);
          impl->work_queue->push_work([conn_data]() {
            uv_close(reinterpret_cast<uv_handle_t *>(&conn_data->conn),
                     on_close_cb);
          });
        }
        continue;
      }
      active_interpreters++;

      auto output_opt = entry.second->output_buffer.pop();
      collection->signals->wake_up_interpreter.notify();
      if (output_opt.has_value()) {
        blocked_interpreters++;
        std::string output = output_opt.value();
        impl->work_queue->push_work([conn_data, output, collection]() {
          uv_write_t *req = new uv_write_t;
          req->data = conn_data;
          uv_buf_t wrbuf =
              uv_buf_init(const_cast<char *>(output.data()), output.size());
          uv_write(req, reinterpret_cast<uv_stream_t *>(&conn_data->conn),
                   &wrbuf, 1, on_write_completed);
        });
      }
    }
    if (active_interpreters == 0) {
      if (impl->exit_when_done.load()) {
        impl->server_stopped.set_value();
        return;
      }
    }
    if (blocked_interpreters == 0) {
      collection->signals->wake_up_for_output.wait();
    }
  }
}

// New static free function to allocate sbuffer.
static void server_alloc_buffer_cb(uv_handle_t *handle, size_t suggested,
                                   uv_buf_t *buf) {
  buf->base = static_cast<char *>(malloc(suggested));
  buf->len = suggested;
}

// New static free function to handle read events.
static void server_on_read_cb(uv_stream_t *stream, ssize_t nread,
                              const uv_buf_t *buf) {
  UvConnectionData *data = static_cast<UvConnectionData *>(stream->data);
  auto collection = data->runner->mgr_->get_collection();
  auto it = collection->interpreters.find(data->fd);
  if (it == collection->interpreters.end()) {
    if (buf->base)
      free(buf->base);
    uv_read_stop(stream);
    uv_close(reinterpret_cast<uv_handle_t *>(stream), on_close_cb);
    return;
  }
  if (nread > 0) {
    std::string input(buf->base, nread);
    it->second->input_buffer.push_back(input);
    collection->signals->wake_up_for_input.notify();
    collection->signals->wake_up_interpreter.notify();
  } else if (nread < 0) {
    it->second->eof.store(true);
    collection->signals->wake_up_for_input.notify();
    collection->signals->wake_up_interpreter.notify();
  }
  if (buf->base)
    free(buf->base);
}

// New connection callback for the server.
static void on_new_connection_cb(uv_stream_t *server, int status) {
  if (status != 0)
    return;
  auto *handle_data = static_cast<UvConnectionData *>(server->data);
  UvConnectionData *conn_data =
      new UvConnectionData{handle_data->runner, false, uv_tcp_t(), -1};
  uv_tcp_init(conn_data->runner->loop_, &conn_data->conn);
  if (uv_accept(server, reinterpret_cast<uv_stream_t *>(&conn_data->conn)) ==
      0) {
    uv_os_fd_t fd;
    if (uv_fileno(reinterpret_cast<uv_handle_t *>(&conn_data->conn), &fd) ==
        0) {
      conn_data->conn.data = conn_data;
      conn_data->fd = fd;
      // Insert interpreter for this connection.
      conn_data->runner->mgr_->insert_interpreter(static_cast<int>(fd),
                                                  conn_data->runner->program,
                                                  std::nullopt, conn_data);
      uv_read_start(reinterpret_cast<uv_stream_t *>(&conn_data->conn),
                    server_alloc_buffer_cb, server_on_read_cb);
      auto collection = conn_data->runner->mgr_->get_collection();
      collection->signals->wake_up_for_input.notify();
      collection->signals->wake_up_interpreter.notify();
      collection->signals->wake_up_for_output.notify();
      collection->signals->wake_up_for_callback.notify();
    } else {
      uv_close(reinterpret_cast<uv_handle_t *>(&conn_data->conn), on_close_cb);
    }
  } else {
    uv_close(reinterpret_cast<uv_handle_t *>(&conn_data->conn), on_close_cb);
  }
}

} // namespace

void LibuvServerRunner::stop_accepting() {
  // Defer the uv_close call to the loop thread.
  if (impl_->exit_when_done.load())
    return;
  impl_->work_queue->push_work([this]() {
    uv_close(reinterpret_cast<uv_handle_t *>(&impl_->server_), on_close_cb);
    impl_->exit_when_done.store(true);
    impl_->mgr_->get_collection()->signals->wake_up_for_output.notify();
  });
}

LibuvServerRunner::LibuvServerRunner(
    networkprotocoldsl::InterpreterCollectionManager &mgr, uv_loop_t *loop,
    const std::string &ip, int port, const InterpretedProgram &program,
    networkprotocoldsl_uv::AsyncWorkQueue &async_queue) { // new parameter
  bind_result = std::promise<BindResult>();
  impl_ =
      new LibuvServerRunnerImpl{&mgr, loop, uv_tcp_t(), program, &async_queue};
  server_stopped = impl_->server_stopped.get_future();
  // Wrap uv_tcp_init call so it runs in the loop thread.
  async_queue.push_work([this, loop, ip, port, program, &async_queue]() {
    uv_tcp_init(loop, &impl_->server_);
    // Create a new merged context.
    // (Deferring the bind operation to the loop thread)
    auto *ctx = new UvConnectionData{impl_, false, uv_tcp_t(), -1};
    ctx->runner = impl_;
    impl_->server_.data = ctx;

    struct sockaddr_in bind_addr;
    uv_ip4_addr(ip.c_str(), port, &bind_addr);
    int rc =
        uv_tcp_bind(&impl_->server_,
                    reinterpret_cast<const struct sockaddr *>(&bind_addr), 0);
    if (rc != 0) {
      bind_result.set_value(std::string(uv_strerror(rc)));
      return;
    }
    uv_os_fd_t fd;
    if (uv_fileno(reinterpret_cast<uv_handle_t *>(&impl_->server_), &fd) == 0) {
      bind_result.set_value(static_cast<int>(fd));
      uv_listen(reinterpret_cast<uv_stream_t *>(&impl_->server_), 1024,
                on_new_connection_cb);
    } else {
      bind_result.set_value(
          "Failed to retrieve file descriptor after binding.");
    }
    // Spawn a single, global pusher thread.
    ctx->runner->pusher_thread =
        std::thread(pusher_thread_loop_all, ctx->runner);
  });
}

// destructor
LibuvServerRunner::~LibuvServerRunner() {
  stop_accepting();
  if (impl_->pusher_thread.joinable()) {
    impl_->pusher_thread.join();
  }
  delete impl_;
}

} // namespace networkprotocoldsl_uv