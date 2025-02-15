#include <gtest/gtest.h>

#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/interpretercollectionmanager.hpp>
#include <networkprotocoldsl/lexer/token.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>
#include <networkprotocoldsl/parser/parse.hpp>
#include <networkprotocoldsl/print_optreenode.hpp>
#include <networkprotocoldsl/sema/analyze.hpp>
#include <networkprotocoldsl/value.hpp>
#include <networkprotocoldsl_uv/asyncworkqueue.hpp>
#include <networkprotocoldsl_uv/libuvclientrunner.hpp>
#include <networkprotocoldsl_uv/libuvclientwrapper.hpp>
#include <networkprotocoldsl_uv/libuvserverwrapper.hpp>

#include <uv.h>

#include <array>
#include <atomic>
#include <future>
#include <memory>
#include <optional>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <variant>

#define NUMBER_OF_CLIENTS_IN_TEST 5

using namespace networkprotocoldsl;
using namespace networkprotocoldsl_uv;

value::Octets _o(const std::string &str) {
  return value::Octets{std::make_shared<std::string>(str)};
}

TEST(LibuvIORunnerTest, Complete) {
  std::string test_file =
      std::string(TEST_DATA_DIR) + "/023-source-code-http-client-server.txt";

  // Create libuv loop and initialize the LibuvIORunner.
  uv_loop_t *loop = uv_default_loop();
  // Create an async work queue for both client and server.
  networkprotocoldsl_uv::AsyncWorkQueue async_queue(loop);

  std::thread io_thread([&]() { uv_run(loop, UV_RUN_DEFAULT); });

  // Generate server program, manager and runner
  auto maybe_server_program = InterpretedProgram::generate_server(test_file);
  ASSERT_TRUE(maybe_server_program.has_value());
  auto server_program = maybe_server_program.value();

  InterpreterRunner::callback_map server_callbacks = {
      {"AwaitResponse",
       [](const std::vector<Value> &args) -> Value {
         value::Dictionary dict = std::get<value::Dictionary>(args[0]);
         std::cerr << "Member count: " << dict.members->size() << std::endl;
         std::cerr << "Server received request: "
                   << std::get<value::Octets>(
                          dict.members->at("request_target"))
                          .data.get()
                   << std::endl;
         return value::DynamicList{
             {_o("HTTP Response"),
              value::Dictionary{
                  {{"response_code", 200},
                   {"reason_phrase", _o("Looks good")},
                   {"major_version", 1},
                   {"minor_version", 1},
                   {"headers",
                    value::DynamicList{
                        {value::Dictionary{{{"key", _o("Some-Response")},
                                            {"value", _o("some value")}}},
                         value::Dictionary{{{"key", _o("TestHeader")},
                                            {"value", _o("Value")}}}}}}}}}};
       }},
      {"Closed",
       [](const std::vector<Value> &args) -> Value {
         std::cerr << "Server Close" << std::endl;
         return value::DynamicList{{_o("N/A"), args.at(0)}};
       }},
  };

  // Create the server wrapper with the callback map.
  LibuvServerWrapper server_wrapper(server_program, server_callbacks,
                                    async_queue);

  // Wait for the bind result before proceeding.
  std::future<LibuvServerRunner::BindResult> bind_future =
      server_wrapper.start("127.0.0.1", 8080);
  bind_future.wait();
  auto bind_result = bind_future.get();
  if (std::holds_alternative<std::string>(bind_result)) {
    std::cerr << "Bind failed: " << std::get<std::string>(bind_result)
              << std::endl;
  }
  ASSERT_EQ(std::holds_alternative<int>(bind_result), true);
  // the bind result is the file descriptor of the server
  ASSERT_NE(std::get<int>(bind_result), 0);

  // Generate client program.
  auto maybe_client_program = InterpretedProgram::generate_client(test_file);
  ASSERT_TRUE(maybe_client_program.has_value());
  auto client_program = maybe_client_program.value();

  // Create client wrapper with a local callback map.
  networkprotocoldsl::InterpreterRunner::callback_map client_callbacks = {
      {"Open",
       [](const std::vector<Value> &args) -> Value {
         // ...existing callback code...
         static int first_time = 0;
         if (!first_time) {
           std::cerr << "Client Open" << std::endl;
           first_time = 1;
         } else {
           std::cerr << "Back to open!" << std::endl;
           return value::DynamicList{
               {_o("Client Closes Connection"), value::Dictionary{}}};
         }
         return value::DynamicList{
             {_o("HTTP Request"),
              value::Dictionary{
                  {{"verb", _o("GET")},
                   {"request_target", _o("/test")},
                   {"major_version", 1},
                   {"minor_version", 1},
                   {"headers",
                    value::DynamicList{
                        {value::Dictionary{{{"key", _o("Accept")},
                                            {"value", _o("application/json")}}},
                         value::Dictionary{{{"key", _o("TestHeader")},
                                            {"value", _o("Value")}}}}}}}}}};
       }},
      {"Closed",
       [](const std::vector<Value> &args) -> Value {
         std::cerr << "Client Close" << std::endl;
         return value::DynamicList{{_o("N/A"), args.at(0)}};
       }},
  };

  // Create the client wrapper.
  networkprotocoldsl_uv::LibuvClientWrapper client_wrapper(
      client_program, client_callbacks, async_queue);

  // Start connecting.
  auto connection_future = client_wrapper.start("127.0.0.1", 8080);
  connection_future.wait();
  auto connection_result_value = connection_future.get();
  if (std::holds_alternative<std::string>(connection_result_value)) {
    std::cerr << "Connection failed: "
              << std::get<std::string>(connection_result_value) << std::endl;
  }
  ASSERT_EQ(std::holds_alternative<int>(connection_result_value), true);
  int client_fd = std::get<int>(connection_result_value);
  ASSERT_NE(client_fd, 0);

  // Wait for the interpreter result.
  auto &client_interpreter_result = client_wrapper.result();
  client_interpreter_result.wait();
  // Validate the result.
  ASSERT_EQ(std::holds_alternative<value::Dictionary>(
                client_interpreter_result.get()),
            true);

  // Stop the server.
  server_wrapper.stop();

  // de-register the async handle to stop the loop
  async_queue.shutdown().wait();
  io_thread.join();

  // ...existing assertions to validate responses...
}
