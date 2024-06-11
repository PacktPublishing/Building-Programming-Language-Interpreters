#include <networkprotocoldsl/continuation.hpp>
#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/operation/dynamiclist.hpp>
#include <networkprotocoldsl/operation/functioncall.hpp>
#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/optree.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cstring>
#include <gtest/gtest.h>
#include <sstream>
#include <variant>

#include "testlibs/http_message_optrees.hpp"

TEST(parse_and_write_http_message, complete) {
  using namespace networkprotocoldsl;
  operation::FunctionCall function_call;
  operation::DynamicList dynamic_list;
  auto main_optree = std::make_shared<OpTree>(
      OpTree({{function_call,
               {{testlibs::get_write_request_callable(), {}},
                {function_call,
                 {{testlibs::get_read_request_callable(), {}},
                  {dynamic_list, {}}}}}}}));
  InterpretedProgram p(main_optree);
  Interpreter i = p.get_instance();

  std::string input = "GET /foo/bar/baz HTTP/1.1\r\n"
                      "Accept: application/json\r\n"
                      "Host: Test Value\r\n"
                      "\r\n";
  auto input_cursor = input.cbegin();
  std::stringstream output_message_stream(std::ios_base::out);

  while (true) {
    ContinuationState s = i.step();
    if (s == ContinuationState::Blocked) {
      auto reason = std::get<ReasonForBlockedOperation>(i.get_result());
      if (reason == ReasonForBlockedOperation::WaitingForWrite) {
        auto buffer = i.get_write_buffer();
        output_message_stream.write(buffer.cbegin(), buffer.size());
        i.handle_write(buffer.size());
      } else if (reason == ReasonForBlockedOperation::WaitingForRead) {
        ASSERT_NE(input_cursor, input.cend());
        size_t movement =
            i.handle_read(std::string_view{input_cursor, input.cend()});
        int buffer_left = std::distance(input_cursor, input.cend());
        ASSERT_GT(buffer_left, 0);
        if (movement <= static_cast<size_t>(buffer_left)) {
          std::advance(input_cursor, movement);
        } else {
          input_cursor = input.cend();
        }
      }
    } else if (s == ContinuationState::Exited) {
      break;
    }
  }

  ASSERT_EQ(input, output_message_stream.str());
}
