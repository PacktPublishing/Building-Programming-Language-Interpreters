#include <networkprotocoldsl/lexer/token.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>
#include <networkprotocoldsl/parser/parse.hpp>

#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <vector>

using namespace networkprotocoldsl;

TEST(MessageTest, Message) {
  std::string test_file =
      std::string(TEST_DATA_DIR) + "/023-source-code-http-client-server.txt";
  std::ifstream file(test_file);
  ASSERT_TRUE(file.is_open());
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();
  auto maybe_tokens = lexer::tokenize(content);
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result = parser::parse(tokens);
  ASSERT_TRUE(result.has_value());
  auto protocol_description = result.value();
  ASSERT_EQ(3, protocol_description->size());

  auto req = protocol_description->at("HTTP Request");
  ASSERT_EQ(8, req->parts->size());
  ASSERT_EQ(6, req->data->size());
  ASSERT_EQ("HTTP Request", req->name->value);
  ASSERT_EQ("Open", req->when->name);
  ASSERT_EQ("AwaitResponse", req->then->name);
  ASSERT_EQ("Client", req->agent->name);

  auto res = protocol_description->at("HTTP Response");
  ASSERT_EQ(8, res->parts->size());
  ASSERT_EQ(6, res->data->size());
  ASSERT_EQ("HTTP Response", res->name->value);
  ASSERT_EQ("AwaitResponse", res->when->name);
  ASSERT_EQ("Open", res->then->name);
  ASSERT_EQ("Server", res->agent->name);

  auto open = protocol_description->at("Client Closes Connection");
  ASSERT_EQ(0, open->parts->size());
  ASSERT_EQ(0, open->data->size());
  ASSERT_EQ("Client Closes Connection", open->name->value);
  ASSERT_EQ("Open", open->when->name);
  ASSERT_EQ("Closed", open->then->name);
}
