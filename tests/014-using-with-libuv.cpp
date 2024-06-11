#include <networkprotocoldsl/continuation.hpp>
#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/interpretercollectionmanager.hpp>
#include <networkprotocoldsl/interpreterrunner.hpp>
#include <networkprotocoldsl/operation/dynamiclist.hpp>
#include <networkprotocoldsl/operation/functioncall.hpp>
#include <networkprotocoldsl/operation/lexicalpadget.hpp>
#include <networkprotocoldsl/operation/opsequence.hpp>
#include <networkprotocoldsl/operation/unarycallback.hpp>
#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/optree.hpp>
#include <networkprotocoldsl/support/mutexlockqueue.hpp>
#include <networkprotocoldsl/support/transactionalcontainer.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <variant>

#include "testlibs/http_message_optrees.hpp"

#include <uv.h>

#define NUMBER_OF_CLIENTS_IN_TEST 5

using namespace networkprotocoldsl;

struct OutputRunner {
  std::atomic<bool> exit_when_done = false;
  void run(InterpreterCollectionManager &mgr);
};

struct UvIntegrationData {
  InterpreterCollectionManager mgr;
  InterpretedProgram server_program;
  InterpretedProgram client_program;
  InterpreterRunner irun;
  OutputRunner orun;
  uv_loop_t *loop;
  uv_tcp_t server;
  std::atomic<int> client_counter;
  int client_start_counter;
  support::MutexLockQueue<std::future<Value>> responses;
  support::MutexLockQueue<std::function<void()>> work_on_loop_thread;
  uv_async_t wake_up_for_loop_thread_work;
  struct sockaddr_in connect_addr;
};

struct UvConnectionData {
  UvIntegrationData *uv_data;
  uv_tcp_t conn;
  int fd;
  int client_number;
};

static void on_close_connection(uv_handle_t *handle) {
  UvConnectionData *conn_data = (UvConnectionData *)handle->data;
  UvIntegrationData *uv_data = conn_data->uv_data;
  conn_data->uv_data->work_on_loop_thread.push_back([uv_data, conn_data] {
    auto collection = uv_data->mgr.get_collection();
    collection->signals->wake_up_for_input.notify();
    collection->signals->wake_up_for_output.notify();
    collection->signals->wake_up_for_callback.notify();
    collection->signals->wake_up_interpreter.notify();
    delete conn_data;
  });
  uv_async_send(&conn_data->uv_data->wake_up_for_loop_thread_work);
}

static void on_write_finished(uv_write_t *req, int status) {
  std::optional<std::string> *cbdata = (std::optional<std::string> *)req->data;
  delete cbdata;
  delete req;
}

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size,
                         uv_buf_t *buf) {
  buf->base = (char *)malloc(suggested_size);
  buf->len = suggested_size;
}

void OutputRunner::run(InterpreterCollectionManager &mgr) {
  int active_interpreters = 0;
  while (true) {
    int output_count = 0;
    active_interpreters = 0;
    auto collection = mgr.get_collection();
    for (auto &[fd, context] : collection->interpreters) {
      UvConnectionData *conn_data =
          (UvConnectionData *)context->additional_data;
      if (context->exited.load()) {
        conn_data->uv_data->work_on_loop_thread.push_back([conn_data] {
          conn_data->conn.data = conn_data;
          uv_close((uv_handle_t *)&conn_data->conn, on_close_connection);
        });
        uv_async_send(&conn_data->uv_data->wake_up_for_loop_thread_work);
        mgr.remove_interpreter(fd);
        continue;
      }
      active_interpreters++;
      auto cbdata =
          new std::optional<std::string>(context->output_buffer.pop());
      if (cbdata->has_value()) {
        output_count++;
        conn_data->uv_data->work_on_loop_thread.push_back([conn_data, cbdata] {
          uv_write_t *req = new uv_write_t;
          req->data = cbdata;
          uv_buf_t wrbuf =
              uv_buf_init(cbdata->value().data(), cbdata->value().size());
          uv_write(req, (uv_stream_t *)&conn_data->conn, &wrbuf, 1,
                   on_write_finished);
        });
        uv_async_send(&conn_data->uv_data->wake_up_for_loop_thread_work);
      } else {
        delete cbdata;
      }
    }
    bool loaded_exit_when_done = exit_when_done.load();
    if (mgr.get_collection() == collection) {
      if (active_interpreters == 0 && loaded_exit_when_done) {
        collection->signals->wake_up_for_callback.notify();
        collection->signals->wake_up_for_input.notify();
        collection->signals->wake_up_interpreter.notify();
        break;
      } else if (output_count == 0) {
        collection->signals->wake_up_for_output.wait();
      }
    }
  }
}

void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
  UvConnectionData *data = (UvConnectionData *)client->data;
  auto collection = data->uv_data->mgr.get_collection();
  const auto &interpreters = collection->interpreters;
  auto ctx_it = interpreters.find(data->fd);
  if (ctx_it != interpreters.end()) {
    if (nread > 0) {
      std::string input = std::string(buf->base, nread);
      ctx_it->second->input_buffer.push_back(input);
      collection->signals->wake_up_interpreter.notify();
    } else {
      ctx_it->second->eof.store(true);
    }
  }
  if (buf->base) {
    free(buf->base);
  }
}

static void on_new_connection(uv_stream_t *server, int status) {
  if (status != 0) {
    return;
  }
  UvIntegrationData *uv_data = (UvIntegrationData *)server->data;
  UvConnectionData *conn_data = new UvConnectionData();
  conn_data->uv_data = uv_data;

  uv_tcp_init(uv_data->loop, &conn_data->conn);
  if (uv_accept(server, (uv_stream_t *)(&conn_data->conn)) == 0) {
    uv_os_fd_t fd;
    if (uv_fileno(reinterpret_cast<uv_handle_t *>(&conn_data->conn), &fd) ==
        0) {
      conn_data->conn.data = conn_data;
      conn_data->fd = fd;
      uv_data->mgr.insert_interpreter(static_cast<int>(fd),
                                      uv_data->server_program, std::nullopt,
                                      conn_data);
      uv_read_start((uv_stream_t *)&conn_data->conn, alloc_buffer, on_read);
    } else {
      delete conn_data;
      uv_close((uv_handle_t *)&conn_data->conn, NULL);
    }
  } else {
    delete conn_data;
    uv_close((uv_handle_t *)&conn_data->conn, NULL);
  }
}

static Value get_request_for_client(int number) {
  return value::DynamicList{
      std::make_shared<const std::vector<Value>>(std::initializer_list<Value>{
          value::Octets{std::make_shared<const std::string>("GET")},
          value::Octets{std::make_shared<const std::string>("/test")}, number,
          number,
          value::DynamicList{std::make_shared<const std::vector<Value>>(
              std::initializer_list<Value>{
                  value::DynamicList{std::make_shared<const std::vector<Value>>(
                      std::initializer_list<Value>{
                          value::Octets{
                              std::make_shared<const std::string>("Accept")},
                          value::Octets{std::make_shared<const std::string>(
                              "application/json")},
                      })},
                  value::DynamicList{std::make_shared<const std::vector<Value>>(
                      std::initializer_list<Value>{
                          value::Octets{std::make_shared<const std::string>(
                              "TestHeader")},
                          value::Octets{
                              std::make_shared<const std::string>("Value")},
                      })}})}})};
}

static void on_connect_client(uv_connect_t *client, int status) {
  UvConnectionData *conn_data = (UvConnectionData *)client->data;
  UvIntegrationData *uv_data = conn_data->uv_data;
  if (status < 0) {
    delete conn_data;
    return;
  }

  uv_tcp_t *handle = (uv_tcp_t *)client->handle;
  handle->data = conn_data;

  uv_os_fd_t fd;
  if (uv_fileno((uv_handle_t *)handle, &fd) == 0) {
    int count = uv_data->client_counter.fetch_add(1);
    conn_data->fd = fd;
    uv_data->responses.push_back(uv_data->mgr.insert_interpreter(
        static_cast<int>(fd), uv_data->client_program,
        get_request_for_client(count), conn_data));
    int rc = uv_read_start((uv_stream_t *)handle, alloc_buffer, on_read);
    if (rc != 0) {
      uv_data->mgr.get_collection()->interpreters.at(fd)->eof.store(true);
      fprintf(stderr, "Client %i failed to uv_read_start: %s\n", fd,
              uv_strerror(rc));
    }
  } else {
    uv_close((uv_handle_t *)handle, on_close_connection);
    delete conn_data;
  }
  delete client;
}

static Value handle_request(const std::vector<Value> &request) {
  // let's test that the request writer and request reader did
  // the right thing. we should have a data structure that looks
  // like what was returned in get_request_for_client
  if (request.size() != 1)
    return value::RuntimeError::ProtocolMismatchError;
  Value message = request[0];
  if (!std::holds_alternative<value::DynamicList>(message))
    return value::RuntimeError::ProtocolMismatchError;
  value::DynamicList members = std::get<value::DynamicList>(message);
  if (members.values->size() != 5)
    return value::RuntimeError::ProtocolMismatchError;
  if (!std::holds_alternative<value::Octets>(members.values->at(0)))
    return value::RuntimeError::ProtocolMismatchError;
  if (!std::holds_alternative<value::Octets>(members.values->at(1)))
    return value::RuntimeError::ProtocolMismatchError;
  if (!std::holds_alternative<int32_t>(members.values->at(2)))
    return value::RuntimeError::ProtocolMismatchError;
  if (!std::holds_alternative<int32_t>(members.values->at(3)))
    return value::RuntimeError::ProtocolMismatchError;
  if (!std::holds_alternative<value::DynamicList>(members.values->at(4)))
    return value::RuntimeError::ProtocolMismatchError;
  value::DynamicList headers =
      std::get<value::DynamicList>(members.values->at(4));
  if (headers.values->size() != 2)
    return value::RuntimeError::ProtocolMismatchError;
  for (auto header : *headers.values) {
    if (!std::holds_alternative<value::DynamicList>(header))
      return value::RuntimeError::ProtocolMismatchError;
    value::DynamicList kv = std::get<value::DynamicList>(header);
    if (kv.values->size() != 2)
      return value::RuntimeError::ProtocolMismatchError;
    if (!std::holds_alternative<value::Octets>(kv.values->at(0)))
      return value::RuntimeError::ProtocolMismatchError;
    if (!std::holds_alternative<value::Octets>(kv.values->at(1)))
      return value::RuntimeError::ProtocolMismatchError;
  }
  // we double checked the entire thing, and it matches, now we need to produce
  // a response. we want to use the value that came in that identifies the
  // client number, it's in the major version and the minor version, and it has
  // the same value
  int number = std::get<int32_t>(members.values->at(2));

  // {"major_version", "minor_version", "status_code", "reason_phrase",
  // "headers"}),
  return value::DynamicList{
      std::make_shared<const std::vector<Value>>(std::initializer_list<Value>{
          number, number, 200,
          value::Octets{std::make_shared<const std::string>("Looks good")},
          value::DynamicList{std::make_shared<const std::vector<Value>>(
              std::initializer_list<Value>{
                  value::DynamicList{std::make_shared<const std::vector<Value>>(
                      std::initializer_list<Value>{
                          value::Octets{std::make_shared<const std::string>(
                              "Content-type")},
                          value::Octets{std::make_shared<const std::string>(
                              "application/json")},
                      })},
                  value::DynamicList{std::make_shared<const std::vector<Value>>(
                      std::initializer_list<Value>{
                          value::Octets{std::make_shared<const std::string>(
                              "TestResponse")},
                          value::Octets{std::make_shared<const std::string>(
                              "OtherValue")},
                      })}})}})};
}

static void do_work_on_loop_thread(uv_async_t *handle) {
  UvIntegrationData *data = (UvIntegrationData *)handle->data;
  while (true) {
    auto next = data->work_on_loop_thread.pop();
    if (next.has_value()) {
      next.value()();
    } else {
      break;
    }
  }
}

static void print_handle(uv_handle_t *handle, void *arg) {
  if (uv_is_active(handle)) {
    fprintf(stdout, "Handle type: %s is still active\n",
            uv_handle_type_name(uv_handle_get_type(handle)));
  }
}

static void no_longer_accepting_connections(uv_handle_t *handle) {
  UvIntegrationData *data = (UvIntegrationData *)handle->data;
  data->irun.exit_when_done.store(true);
  data->orun.exit_when_done.store(true);
}

TEST(client_and_server, complete) {
  operation::FunctionCall function_call;
  operation::DynamicList dynamic_list;
  operation::LexicalPadGet get_argv("argv");
  operation::UnaryCallback callback("handle_request");
  operation::OpSequence ops;

  auto server_optree = std::make_shared<OpTree>(OpTree(
      {{function_call,
        {{testlibs::get_write_response_callable(), {}},
         {callback,
          {{function_call,
            {{testlibs::get_read_request_callable(), {}}, {dynamic_list, {}}}}

          }}}}}));
  InterpretedProgram server_program(server_optree);

  auto client_optree = std::make_shared<OpTree>(OpTree(
      {{ops,
        {
            {function_call,
             {{testlibs::get_write_request_callable(), {}}, {get_argv, {}}}},
            {function_call,
             {{testlibs::get_read_response_callable(), {}},
              {dynamic_list, {}}}},

        }}}));
  InterpretedProgram client_program(client_optree);

  // initialize the data we need in the loop
  UvIntegrationData uv_data{InterpreterCollectionManager(),
                            server_program,
                            client_program,
                            InterpreterRunner(),
                            OutputRunner(),
                            uv_default_loop()};

  // initialize the tcp server
  uv_data.server.data = &uv_data;
  uv_tcp_init(uv_data.loop, &uv_data.server);
  // Ignore SIGPIPE
  signal(SIGPIPE, SIG_IGN);

  struct sockaddr_in bind_addr;
  // bind to a random port and keep track of it.
  uv_ip4_addr("127.0.0.1", 0, &bind_addr);
  uv_tcp_bind(&uv_data.server, (const struct sockaddr *)&bind_addr, 0);
  struct sockaddr_in bind_sockname;
  int namelen = sizeof(bind_sockname);
  uv_tcp_getsockname(&uv_data.server, (struct sockaddr *)&bind_sockname,
                     &namelen);

  uv_ip4_addr("127.0.0.1", ntohs(bind_sockname.sin_port),
              &uv_data.connect_addr);

  uv_data.wake_up_for_loop_thread_work.data = &uv_data;
  uv_async_init(uv_data.loop, &uv_data.wake_up_for_loop_thread_work,
                do_work_on_loop_thread);

  // attach the libuv callbacks
  int rc = uv_listen((uv_stream_t *)&uv_data.server, 1024, on_new_connection);
  ASSERT_EQ(rc, 0);

  // starting up the interpreter loop
  uv_data.irun.callbacks.emplace("handle_request", handle_request);
  std::thread interpreter_thread(
      [&uv_data] { uv_data.irun.interpreter_loop(uv_data.mgr); });
  std::thread callbacks_thread(
      [&uv_data] { uv_data.irun.callback_loop(uv_data.mgr); });
  std::thread output_thread([&uv_data] { uv_data.orun.run(uv_data.mgr); });
  std::thread io_thread([&uv_data] { uv_run(uv_data.loop, UV_RUN_DEFAULT); });

  // now that we have a server running and bound to the port, let's run a few
  // clients and accumulate the responses
  for (int i = 0; i < NUMBER_OF_CLIENTS_IN_TEST; i++) {
    uv_data.work_on_loop_thread.push_back([&uv_data] {
      UvConnectionData *data = new UvConnectionData();
      data->uv_data = &uv_data;
      data->client_number = uv_data.client_start_counter++;
      uv_tcp_init(uv_data.loop, &data->conn);

      uv_connect_t *connect_req = new uv_connect_t;
      connect_req->data = data;
      int rc = uv_tcp_connect(connect_req, &data->conn,
                              (const struct sockaddr *)&uv_data.connect_addr,
                              on_connect_client);
      if (rc != 0) {
        fprintf(stderr, "Failed to request connection: %s\n", uv_strerror(rc));
      }
    });
    uv_async_send(&uv_data.wake_up_for_loop_thread_work);
  }

  // make sure we got all the connections
  while (uv_data.client_counter.load() < NUMBER_OF_CLIENTS_IN_TEST) {
    usleep(100);
  }

  // finalize the interpreter loop
  uv_data.work_on_loop_thread.push_back([&uv_data] {
    uv_close((uv_handle_t *)&(uv_data.server), no_longer_accepting_connections);
  });
  uv_async_send(&uv_data.wake_up_for_loop_thread_work);

  auto signals = uv_data.mgr.get_collection()->signals;
  signals->wake_up_interpreter.notify();
  signals->wake_up_for_callback.notify();
  signals->wake_up_for_output.notify();

  interpreter_thread.join();
  output_thread.join();
  callbacks_thread.join();

  // Now only the io loop is there, let's close the async handler and check if
  // anything else is open
  uv_data.work_on_loop_thread.push_back([&uv_data] {
    uv_close((uv_handle_t *)&(uv_data.wake_up_for_loop_thread_work), NULL);
    uv_walk(uv_data.loop, print_handle, NULL);
  });
  uv_async_send(&uv_data.wake_up_for_loop_thread_work);

  io_thread.join();

  // Now we're going to check all the responses.
  std::array<bool, NUMBER_OF_CLIENTS_IN_TEST> collected_responses = {};
  int failed_count = 0;
  while (true) {
    auto response_future = uv_data.responses.pop();
    if (!response_future.has_value())
      break;
    auto message = response_future.value().get();
    // let's check the entire response
    // {"major_version", "minor_version", "status_code", "reason_phrase",
    // "headers"}),
    if (std::holds_alternative<value::RuntimeError>(message)) {
      std::cerr << "One request failed." << std::endl;
      failed_count++;
      continue;
    }
    ASSERT_TRUE(std::holds_alternative<value::DynamicList>(message));
    value::DynamicList members = std::get<value::DynamicList>(message);
    ASSERT_EQ(members.values->size(), 5);
    ASSERT_TRUE(std::holds_alternative<int32_t>(members.values->at(0)));
    ASSERT_TRUE(std::holds_alternative<int32_t>(members.values->at(1)));
    ASSERT_TRUE(std::holds_alternative<int32_t>(members.values->at(2)));
    ASSERT_TRUE(std::holds_alternative<value::Octets>(members.values->at(3)));
    ASSERT_TRUE(
        std::holds_alternative<value::DynamicList>(members.values->at(4)));
    value::DynamicList headers =
        std::get<value::DynamicList>(members.values->at(4));
    ASSERT_EQ(2, headers.values->size());
    for (auto header : *headers.values) {
      ASSERT_TRUE(std::holds_alternative<value::DynamicList>(header));
      value::DynamicList kv = std::get<value::DynamicList>(header);
      ASSERT_EQ(2, kv.values->size());
      ASSERT_TRUE(std::holds_alternative<value::Octets>(kv.values->at(0)));
      ASSERT_TRUE(std::holds_alternative<value::Octets>(kv.values->at(1)));
    }
    // the client number was sent over the major version and the minor version
    collected_responses[std::get<int32_t>(members.values->at(0))] = true;
  }
  for (auto r : collected_responses) {
    ASSERT_TRUE(r || failed_count--); // should have seen for all requests
  }
}
