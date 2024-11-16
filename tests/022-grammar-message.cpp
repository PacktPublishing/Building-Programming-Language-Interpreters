#include <networkprotocoldsl/lexer/token.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>
#include <networkprotocoldsl/parser/grammar/message.hpp>

#include <gtest/gtest.h>
#include <memory>
#include <vector>

using namespace networkprotocoldsl;

TEST(MessageTest, Message) {
  auto maybe_tokens = lexer::tokenize(
      "message \"HTTP Request\" { "
      "    when: Open; "
      "    then: AwaitResponse; "
      "    agent: Client; "
      "    data: { "
      "        method: str<encoding=Ascii7Bit, "
      "                    sizing=Dynamic, "
      "                    max_length=10>; "
      "        request_target: str<encoding=Ascii7Bit, "
      "                            sizing=Dynamic, "
      "                            max_length=32768>; "
      "        major_version: int<encoding=AsciiInt, "
      "                           unsigned=True, "
      "                           bits=8>; "
      "        minor_version: int<encoding=AsciiInt, "
      "                           unsigned=True, "
      "                           bits=8>; "
      "        headers: array< "
      "          element_type=tuple<key=str<encoding=Ascii7Bit, "
      "                                     sizing=Dynamic, "
      "                                     max_length=32768>, "
      "                             value=str<encoding=Ascii7Bit, "
      "                                       sizing=Dynamic, "
      "                                       max_length=32768>>, "
      "          sizing=Dynamic, "
      "          max_length=100>; "
      "        contents: stream; "
      "    } "
      "    parts { "
      "        tokens { verb } "
      "        terminator { \" \" } "
      "        tokens { request_target } "
      "        terminator { \" \" } "
      "        tokens { "
      "            \"HTTP/\" major_version \".\" minor_version "
      "        } "
      "        terminator { \"\\r\\n\" } "
      "        for header in headers { "
      "            tokens { header.key } "
      "            terminator { \":\" } "
      "            tokens { header.value } "
      "            terminator { \"\\r\\n\" } "
      "        } "
      "        terminator { \"\\r\\n\" } "
      "    } "
      "} ");
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result = parser::grammar::Message::parse(tokens.cbegin(), tokens.cend());
  ASSERT_EQ(result.begin, tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  auto message = std::get<std::shared_ptr<const parser::tree::Message>>(
      result.node.value());
  ASSERT_EQ(8, message->parts->size());
  ASSERT_EQ(6, message->data->size());
  ASSERT_EQ("HTTP Request", message->name->value);
  ASSERT_EQ("Open", message->when->name);
  ASSERT_EQ("AwaitResponse", message->then->name);
  ASSERT_EQ("Client", message->agent->name);
}
