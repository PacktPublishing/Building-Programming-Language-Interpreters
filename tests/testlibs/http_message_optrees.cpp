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

namespace {
using namespace networkprotocoldsl;

operation::LexicalPadInitialize init_verb("verb");
operation::LexicalPadInitialize init_request_target("request_target");
operation::LexicalPadInitialize init_major_version("major_version");
operation::LexicalPadInitialize init_minor_version("minor_version");
operation::LexicalPadInitialize init_headers("headers");
operation::LexicalPadInitialize init_key("key");
operation::LexicalPadInitialize init_vaklue("value");
operation::LexicalPadInitialize init_status_code("status_code");
operation::LexicalPadInitialize init_reason_phrase("reason_phrase");

operation::ReadOctetsUntilTerminator read_until_space(" ");
operation::ReadOctetsUntilTerminator read_until_colon(":");
operation::ReadOctetsUntilTerminator read_until_crlf("\r\n");

operation::ReadStaticOctets read_http_slash("HTTP/");
operation::ReadStaticOctets read_dot(".");
operation::ReadStaticOctets read_crlf("\r\n");
operation::ReadStaticOctets read_space(" ");

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
operation::LexicalPadGet get_status_code("status_code");
operation::LexicalPadGet get_reason_phrase("reason_phrase");

operation::WriteOctets write_octets;

operation::WriteStaticOctets write_space(" ");
operation::WriteStaticOctets write_http_slash("HTTP/");
operation::WriteStaticOctets write_dot(".");
operation::WriteStaticOctets write_crlf("\r\n");
operation::WriteStaticOctets write_colon(":");

operation::IntToAscii int_to_ascii;

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

} // namespace
namespace testlibs {
networkprotocoldsl::Operation get_read_request_callable() {
  return operation::StaticCallable(std::make_shared<OpTree>(OpTree(
      {{ops,
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
  ;
}
networkprotocoldsl::Operation get_read_response_callable() {
  return operation::StaticCallable(std::make_shared<OpTree>(OpTree(
      {{ops,
        {{read_http_slash, {}},
         {init_major_version, {{read_atoi, {}}}},
         {read_dot, {}},
         {init_minor_version, {{read_atoi, {}}}},
         {read_space, {}},
         {init_status_code, {{read_atoi, {}}}},
         {read_space, {}},
         {init_reason_phrase, {{read_until_crlf, {}}}},
         {init_headers, {{generate_list, {{read_headers_callable, {}}}}}},
         {dynamic_list,
          {{get_major_version, {}},
           {get_minor_version, {}},
           {get_status_code, {}},
           {get_reason_phrase, {}},
           {get_headers, {}}}}}}})));
  ;
}
networkprotocoldsl::Operation get_write_request_callable() {
  return operation::StaticCallable(
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
}
networkprotocoldsl::Operation get_write_response_callable() {
  return operation::StaticCallable(
      std::make_shared<OpTree>(
          OpTree({{ops,
                   {{write_http_slash, {}},
                    {write_octets, {{int_to_ascii, {{get_major_version, {}}}}}},
                    {write_dot, {}},
                    {write_octets, {{int_to_ascii, {{get_minor_version, {}}}}}},
                    {write_space, {}},
                    {write_octets, {{int_to_ascii, {{get_status_code, {}}}}}},
                    {write_space, {}},
                    {write_octets, {{get_reason_phrase, {}}}},
                    {write_crlf, {}},
                    {function_call_for_each,
                     {{write_headers_callable, {}}, {get_headers, {}}}},
                    {write_crlf, {}}}}})),
      std::vector<std::string>({"major_version", "minor_version", "status_code",
                                "reason_phrase", "headers"}),
      false);
}
} // namespace testlibs
