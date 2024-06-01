#include <networkprotocoldsl/continuation.hpp>
#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/operation/dynamiclist.hpp>
#include <networkprotocoldsl/operation/functioncall.hpp>
#include <networkprotocoldsl/operation/functioncallforeach.hpp>
#include <networkprotocoldsl/operation/generatelist.hpp>
#include <networkprotocoldsl/operation/inttoascii.hpp>
#include <networkprotocoldsl/operation/lexicalpadget.hpp>
#include <networkprotocoldsl/operation/lexicalpadinitializeglobal.hpp>
#include <networkprotocoldsl/operation/opsequence.hpp>
#include <networkprotocoldsl/operation/readintfromascii.hpp>
#include <networkprotocoldsl/operation/readoctetsuntilterminator.hpp>
#include <networkprotocoldsl/operation/readstaticoctets.hpp>
#include <networkprotocoldsl/operation/staticcallable.hpp>
#include <networkprotocoldsl/operation/terminatelistifreadahead.hpp>
#include <networkprotocoldsl/operation/writeoctets.hpp>
#include <networkprotocoldsl/operation/writestaticoctets.hpp>
#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/optree.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cstring>
#include <gtest/gtest.h>
#include <sstream>
#include <variant>

namespace {
using namespace networkprotocoldsl;

operation::LexicalPadInitialize init_verb("verb");
operation::LexicalPadInitialize init_request_target("request_target");
operation::LexicalPadInitialize init_major_version("major_version");
operation::LexicalPadInitialize init_minor_version("minor_version");
operation::LexicalPadInitialize init_headers("headers");
operation::LexicalPadInitialize init_key("key");
operation::LexicalPadInitialize init_vaklue("value");

operation::ReadOctetsUntilTerminator read_until_space(" ");
operation::ReadOctetsUntilTerminator read_until_colon(":");
operation::ReadOctetsUntilTerminator read_until_crlf("\r\n");

operation::ReadStaticOctets read_http_slash("HTTP/");
operation::ReadStaticOctets read_dot(".");
operation::ReadStaticOctets read_crlf("\r\n");

operation::ReadIntFromAscii read_atoi;

operation::DynamicList dynamic_list;

operation::GenerateList generate_list;

operation::OpSequence ops;

operation::TerminateListIfReadAhead terminate_if_read_ahead_crlf("\r\n");

operation::FunctionCallForEach function_call_for_each;

operation::LexicalPadGet get_verb("verb");
operation::LexicalPadGet get_request_target("request_target");
operation::LexicalPadGet get_major_version("major_version");
operation::LexicalPadGet get_minor_version("minor_version");
operation::LexicalPadGet get_headers("headers");
operation::LexicalPadGet get_key("key");
operation::LexicalPadGet get_value("value");

operation::WriteOctets write_octets;

operation::WriteStaticOctets write_space(" ");
operation::WriteStaticOctets write_http_slash("HTTP/");
operation::WriteStaticOctets write_dot(".");
operation::WriteStaticOctets write_crlf("\r\n");
operation::WriteStaticOctets write_colon(":");

operation::IntToAscii int_to_ascii;

operation::FunctionCall function_call;

operation::StaticCallable read_headers_callable(std::make_shared<OpTree>(OpTree(
    {{ops,
      {{terminate_if_read_ahead_crlf, {}},
       {dynamic_list, {{read_until_colon, {}}, {read_until_crlf, {}}}}}}})));

operation::StaticCallable write_headers_callable(
    std::make_shared<OpTree>(OpTree({{ops,
                                      {{write_octets,
                                        {
                                            {get_key, {}},
                                        }},
                                       {write_colon, {}},
                                       {write_octets, {{get_value, {}}}},
                                       {write_crlf, {}}}}})),
    std::vector<std::string>({"key", "value"}), false);

operation::StaticCallable read_message(std::make_shared<OpTree>(
    OpTree({{ops,
             {{init_verb, {{read_until_space, {}}}},
              {init_request_target, {{read_until_space, {}}}},
              {read_http_slash, {}},
              {init_major_version, {{read_atoi, {}}}},
              {read_dot, {}},
              {init_minor_version, {{read_atoi, {}}}},
              {read_crlf, {}},
              {init_headers, {{generate_list, {{read_headers_callable, {}}}}}},
              {dynamic_list,
               {{get_verb, {}},
                {get_request_target, {}},
                {get_major_version, {}},
                {get_minor_version, {}},
                {get_headers, {}}}}}}})));

operation::StaticCallable write_message(
    std::make_shared<OpTree>(
        OpTree({{ops,
                 {{write_octets, {{get_verb, {}}}},
                  {write_space, {}},
                  {write_octets,
                   {
                       {get_request_target, {}},

                   }},
                  {write_space, {}},
                  {write_http_slash, {}},
                  {write_octets, {{int_to_ascii, {{get_major_version, {}}}}}},
                  {write_dot, {}},
                  {write_octets, {{int_to_ascii, {{get_minor_version, {}}}}}},
                  {write_crlf, {}},
                  {function_call_for_each,
                   {{write_headers_callable, {}}, {get_headers, {}}}},
                  {write_crlf, {}}}}})),
    std::vector<std::string>({"verb", "request_target", "major_version",
                              "minor_version", "headers"}),
    false);

auto main_optree = std::make_shared<OpTree>(
    OpTree({{function_call,
             {{write_message, {}},
              {function_call, {{read_message, {}}, {dynamic_list, {}}}}}}}));
} // namespace

TEST(parse_and_write_http_message, complete) {

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
