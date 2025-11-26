#include <networkprotocoldsl_uv/generatedserverwrapper.hpp>

#include <atomic>
#include <cstring>
#include <mutex>
#include <unordered_map>

namespace networkprotocoldsl_uv {

// ============================================================================
// Implementation details
// ============================================================================

struct ConnectionData {
  GeneratedServerWrapperBase::Impl *impl;
  uv_tcp_t handle;
  int fd;
  std::unique_ptr<IConnectionRunner> runner;
  std::atomic<bool> closing{false};
};

struct ServerData {
  GeneratedServerWrapperBase::Impl *impl;
  uv_tcp_t handle;
};

struct GeneratedServerWrapperBase::Impl {
  ConnectionRunnerFactory runner_factory;
  AsyncWorkQueue *async_queue;
  uv_loop_t *loop;

  std::unique_ptr<ServerData> server_data;
  std::promise<BindResult> bind_promise;
  std::promise<void> stopped_promise;
  std::future<void> stopped_future;

  std::mutex connections_mutex;
  std::unordered_map<int, std::unique_ptr<ConnectionData>> connections;

  std::atomic<bool> stopping{false};

  Impl(ConnectionRunnerFactory factory, AsyncWorkQueue &queue)
      : runner_factory(std::move(factory)), async_queue(&queue),
        loop(uv_default_loop()), stopped_future(stopped_promise.get_future()) {}

  void process_output(ConnectionData *conn);
};

// ============================================================================
// Static callbacks
// ============================================================================

namespace {

void on_close(uv_handle_t *handle) {
  auto *conn = static_cast<ConnectionData *>(handle->data);
  auto *impl = conn->impl;

  std::lock_guard<std::mutex> lock(impl->connections_mutex);
  impl->connections.erase(conn->fd);
}

void on_server_close(uv_handle_t *handle) {
  auto *server_data = static_cast<ServerData *>(handle->data);
  auto *impl = server_data->impl;
  impl->stopped_promise.set_value();
}

void on_write_complete(uv_write_t *req, int status) {
  auto *conn = static_cast<ConnectionData *>(req->data);
  auto *impl = conn->impl;

  // Free the write request and buffer
  delete[] static_cast<char *>(req->bufs[0].base);
  delete req;

  if (status < 0) {
    // Write error, close connection
    if (!conn->closing.exchange(true)) {
      uv_close(reinterpret_cast<uv_handle_t *>(&conn->handle), on_close);
    }
    return;
  }

  // Check for more output or if connection should close
  if (conn->runner->is_closed()) {
    if (!conn->closing.exchange(true)) {
      uv_close(reinterpret_cast<uv_handle_t *>(&conn->handle), on_close);
    }
  } else {
    impl->process_output(conn);
  }
}

void on_alloc_buffer(uv_handle_t *handle, size_t suggested, uv_buf_t *buf) {
  buf->base = static_cast<char *>(malloc(suggested));
  buf->len = suggested;
}

void on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
  auto *conn = static_cast<ConnectionData *>(stream->data);
  auto *impl = conn->impl;

  if (nread > 0) {
    // Feed data to the runner
    std::string_view data(buf->base, nread);
    conn->runner->on_bytes_received(data);

    // Process any output
    impl->process_output(conn);
  } else if (nread < 0) {
    // Connection closed or error
    if (!conn->closing.exchange(true)) {
      uv_close(reinterpret_cast<uv_handle_t *>(&conn->handle), on_close);
    }
  }

  if (buf->base) {
    free(buf->base);
  }
}

void on_new_connection(uv_stream_t *server, int status) {
  auto *server_data = static_cast<ServerData *>(server->data);
  auto *impl = server_data->impl;

  if (status < 0 || impl->stopping) {
    return;
  }

  auto conn = std::make_unique<ConnectionData>();
  conn->impl = impl;

  uv_tcp_init(impl->loop, &conn->handle);
  conn->handle.data = conn.get();

  if (uv_accept(server, reinterpret_cast<uv_stream_t *>(&conn->handle)) == 0) {
    uv_os_fd_t fd;
    if (uv_fileno(reinterpret_cast<uv_handle_t *>(&conn->handle), &fd) == 0) {
      conn->fd = static_cast<int>(fd);

      // Create runner for this connection (references the shared handler)
      conn->runner = impl->runner_factory();
      
      // Start the runner (e.g., send initial greeting)
      conn->runner->start();

      // Store connection
      ConnectionData *conn_ptr = conn.get();
      {
        std::lock_guard<std::mutex> lock(impl->connections_mutex);
        impl->connections[conn->fd] = std::move(conn);
      }

      // Check if there's initial output (e.g., server greeting)
      impl->process_output(conn_ptr);

      // Start reading
      uv_read_start(reinterpret_cast<uv_stream_t *>(&conn_ptr->handle),
                    on_alloc_buffer, on_read);
    } else {
      uv_close(reinterpret_cast<uv_handle_t *>(&conn->handle), nullptr);
    }
  } else {
    uv_close(reinterpret_cast<uv_handle_t *>(&conn->handle), nullptr);
  }
}

} // anonymous namespace

// ============================================================================
// Impl methods
// ============================================================================

void GeneratedServerWrapperBase::Impl::process_output(ConnectionData *conn) {
  while (conn->runner->has_pending_output()) {
    std::string_view output = conn->runner->pending_output();
    size_t len = output.size();

    // Copy data for async write
    char *data = new char[len];
    std::memcpy(data, output.data(), len);

    // Mark bytes as consumed
    conn->runner->bytes_written(len);

    // Create write request
    uv_write_t *req = new uv_write_t;
    req->data = conn;
    uv_buf_t buf = uv_buf_init(data, len);

    uv_write(req, reinterpret_cast<uv_stream_t *>(&conn->handle), &buf, 1,
             on_write_complete);
  }
}

// ============================================================================
// GeneratedServerWrapperBase implementation
// ============================================================================

GeneratedServerWrapperBase::GeneratedServerWrapperBase(
    ConnectionRunnerFactory runner_factory, AsyncWorkQueue &async_queue)
    : impl_(std::make_unique<Impl>(std::move(runner_factory), async_queue)) {}

GeneratedServerWrapperBase::~GeneratedServerWrapperBase() { stop(); }

std::future<BindResult> GeneratedServerWrapperBase::start(const std::string &ip,
                                                          int port) {
  auto future = impl_->bind_promise.get_future();

  impl_->async_queue->push_work([this, ip, port]() {
    impl_->server_data = std::make_unique<ServerData>();
    impl_->server_data->impl = impl_.get();

    uv_tcp_init(impl_->loop, &impl_->server_data->handle);
    impl_->server_data->handle.data = impl_->server_data.get();

    struct sockaddr_in bind_addr;
    uv_ip4_addr(ip.c_str(), port, &bind_addr);

    int rc = uv_tcp_bind(
        &impl_->server_data->handle,
        reinterpret_cast<const struct sockaddr *>(&bind_addr), 0);
    if (rc != 0) {
      impl_->bind_promise.set_value(std::string(uv_strerror(rc)));
      return;
    }

    rc = uv_listen(reinterpret_cast<uv_stream_t *>(&impl_->server_data->handle),
                   128, on_new_connection);
    if (rc != 0) {
      impl_->bind_promise.set_value(std::string(uv_strerror(rc)));
      return;
    }

    uv_os_fd_t fd;
    if (uv_fileno(reinterpret_cast<uv_handle_t *>(&impl_->server_data->handle),
                  &fd) == 0) {
      impl_->bind_promise.set_value(static_cast<int>(fd));
    } else {
      impl_->bind_promise.set_value(std::string("Failed to get file descriptor"));
    }
  });

  return future;
}

void GeneratedServerWrapperBase::stop() {
  if (impl_->stopping.exchange(true)) {
    return; // Already stopping
  }

  impl_->async_queue->push_work([this]() {
    // Close all connections
    {
      std::lock_guard<std::mutex> lock(impl_->connections_mutex);
      for (auto &[fd, conn] : impl_->connections) {
        if (!conn->closing.exchange(true)) {
          uv_close(reinterpret_cast<uv_handle_t *>(&conn->handle), on_close);
        }
      }
    }

    // Close the server
    if (impl_->server_data) {
      uv_close(reinterpret_cast<uv_handle_t *>(&impl_->server_data->handle),
               on_server_close);
    }
  });

  impl_->stopped_future.wait();
}

} // namespace networkprotocoldsl_uv
