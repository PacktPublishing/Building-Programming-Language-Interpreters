#include <fstream>
#include <gtest/gtest.h>
#include <networkprotocoldsl/generate.hpp>
#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/interpretercollectionmanager.hpp>
#include <networkprotocoldsl/interpreterrunner.hpp>
#include <networkprotocoldsl/lexer/token.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>
#include <networkprotocoldsl/parser/parse.hpp>
#include <networkprotocoldsl/print_optreenode.hpp>
#include <networkprotocoldsl/sema/analyze.hpp>
#include <sstream>
#include <thread>

using namespace networkprotocoldsl;

static Value _o(const std::string &in) {
  return value::Octets{std::make_shared<std::string>(in)};
}

TEST(TranslateASTToOptree, Translation) {
  std::string test_file =
      std::string(TEST_DATA_DIR) + "/023-source-code-http-client-server.txt";

  // Initialize an interpreter for both the client and server optrees
  auto maybe_client_program = InterpretedProgram::generate_client(test_file);
  ASSERT_TRUE(maybe_client_program.has_value());
  auto client_program = maybe_client_program.value();
  InterpreterRunner client_runner{
      .callbacks =
          {
              {"Open",
               [](const std::vector<Value> &args) -> Value {
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
                                {value::Dictionary{
                                     {{"key", _o("Accept")},
                                      {"value", _o("application/json")}}},
                                 value::Dictionary{
                                     {{"key", _o("TestHeader")},
                                      {"value", _o("Value")}}}}}}}}}};
               }},
              {"Closed",
               [](const std::vector<Value> &args) -> Value {
                 std::cerr << "Client Close" << std::endl;
                 return value::DynamicList{{_o("N/A"), args.at(0)}};
               }},
          },
      .exit_when_done = true};
  InterpreterCollectionManager client_mgr;
  client_mgr.insert_interpreter(0, client_program);

  auto maybe_server_program = InterpretedProgram::generate_server(test_file);
  ASSERT_TRUE(maybe_server_program.has_value());
  auto server_program = maybe_server_program.value();
  InterpreterRunner server_runner{
      .callbacks =
          {
              {"AwaitResponse",
               [](const std::vector<Value> &args) -> Value {
                 value::Dictionary dict = std::get<value::Dictionary>(args[0]);
                 std::cerr << "Member count: " << dict.members->size()
                           << std::endl;
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
                                {value::Dictionary{
                                     {{"key", _o("Some-Response")},
                                      {"value", _o("some value")}}},
                                 value::Dictionary{
                                     {{"key", _o("TestHeader")},
                                      {"value", _o("Value")}}}}}}}}}};
               }},
              {"Closed",
               [](const std::vector<Value> &args) -> Value {
                 std::cerr << "Server Close" << std::endl;
                 return value::DynamicList{{_o("N/A"), args.at(0)}};
               }},
          },
      .exit_when_done = true};
  InterpreterCollectionManager server_mgr;
  server_mgr.insert_interpreter(0, server_program);

  std::stringstream client_writes;
  std::stringstream server_writes;
  // create a thread that will transfer the data written by the client to the
  // server and vice-versa.
  auto io_thread_code = [](auto &this_mgr, auto &other_mgr, auto &this_writes) {
    while (true) {
      bool all_exited = true;
      auto collection = this_mgr.get_collection();
      for (auto &[fd, context] : collection->interpreters) {
        auto cbdata = context->output_buffer.pop();
        if (cbdata.has_value()) {
          all_exited = false;
          this_writes << cbdata.value();
          auto other_col = other_mgr.get_collection();
          auto other_iter = other_col->interpreters.find(0);
          if (other_iter != other_col->interpreters.end()) {
            std::cerr << "Pushing data to other interpreter: " << cbdata.value()
                      << std::endl;
            other_iter->second->input_buffer.push_back(cbdata.value());
            other_col->signals->wake_up_for_input.notify();
            other_col->signals->wake_up_for_output.notify();
            other_col->signals->wake_up_interpreter.notify();
          }
        } else {
          if (context->exited.load()) {
            auto other_col = other_mgr.get_collection();
            auto other_iter = other_col->interpreters.find(0);
            if (other_iter != other_col->interpreters.end()) {
              auto other_iter = other_col->interpreters.find(0);
              if (other_iter != other_col->interpreters.end()) {
                std::cerr << "Pushing EOF" << std::endl;
                other_iter->second->eof.store(true);
                other_col->signals->wake_up_for_input.notify();
                other_col->signals->wake_up_for_output.notify();
                other_col->signals->wake_up_interpreter.notify();
              }
            }
          } else {
            all_exited = false;
          }
          if (context->eof.load()) {
            collection->signals->wake_up_for_output.notify();
          }
        }
        collection->signals->wake_up_for_output.wait();
      }
      if (all_exited) {
        std::cerr << "All exited" << std::endl;
        break;
      }
    }
  };

  std::thread client_io_thread(
      [&]() { io_thread_code(client_mgr, server_mgr, client_writes); });

  std::thread server_io_thread(
      [&]() { io_thread_code(server_mgr, client_mgr, server_writes); });

  std::thread client_interpreter_thread([&client_runner, &client_mgr] {
    client_runner.interpreter_loop(client_mgr);
  });

  std::thread server_interpreter_thread([&server_runner, &server_mgr] {
    server_runner.interpreter_loop(server_mgr);
  });

  std::thread client_callback_thread([&client_runner, &client_mgr] {
    client_runner.callback_loop(client_mgr);
  });

  std::thread server_callback_thread([&server_runner, &server_mgr] {
    server_runner.callback_loop(server_mgr);
  });

  client_io_thread.join();
  server_io_thread.join();
  client_callback_thread.join();
  server_callback_thread.join();
  client_interpreter_thread.join();
  server_interpreter_thread.join();

  ASSERT_EQ(client_writes.str(), "GET /test HTTP/1.1\r\n"
                                 "Accept:application/json\r\n"
                                 "TestHeader:Value\r\n"
                                 "\r\n");
  ASSERT_EQ(server_writes.str(), "HTTP/1.1 200 Looks good\r\n"
                                 "Some-Response:some value\r\n"
                                 "TestHeader:Value\r\n"
                                 "\r\n");
}
