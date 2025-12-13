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
  ASSERT_EQ(5, req->data->size());
  ASSERT_EQ("HTTP Request", req->name->value);
  ASSERT_EQ("Open", req->when->name);
  ASSERT_EQ("AwaitResponse", req->then->name);
  ASSERT_EQ("Client", req->agent->name);

  auto res = protocol_description->at("HTTP Response");
  ASSERT_EQ(8, res->parts->size());
  ASSERT_EQ(5, res->data->size());
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

TEST(MessageTest, HTTPWithContinuationEscape) {
  std::string test_file =
      std::string(TEST_DATA_DIR) + "/038-http-with-continuation.txt";
  std::ifstream file(test_file);
  ASSERT_TRUE(file.is_open()) << "Could not open " << test_file;
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();
  
  auto maybe_tokens = lexer::tokenize(content);
  ASSERT_TRUE(maybe_tokens.has_value()) << "Tokenization failed";
  
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result = parser::parse(tokens);
  ASSERT_TRUE(result.has_value()) << "Parsing failed";
  
  auto protocol_description = result.value();
  ASSERT_EQ(3, protocol_description->size());

  // Check Request Line message
  auto req = protocol_description->at("Request Line");
  ASSERT_EQ("Request Line", req->name->value);
  ASSERT_EQ("Open", req->when->name);
  ASSERT_EQ("ExpectResponse", req->then->name);
  ASSERT_EQ("Client", req->agent->name);
  ASSERT_EQ(3, req->data->size()); // method, path, host
  
  // Check Response Line message  
  auto res = protocol_description->at("Response Line");
  ASSERT_EQ("Response Line", res->name->value);
  ASSERT_EQ("ExpectResponse", res->when->name);
  ASSERT_EQ("Done", res->then->name);
  ASSERT_EQ("Server", res->agent->name);
  ASSERT_EQ(3, res->data->size()); // status, reason, content_type

  // Check Close Connection message
  auto close = protocol_description->at("Close Connection");
  ASSERT_EQ("Close Connection", close->name->value);
  ASSERT_EQ("Done", close->when->name);
  ASSERT_EQ("Closed", close->then->name);
  ASSERT_EQ("Client", close->agent->name);
}
