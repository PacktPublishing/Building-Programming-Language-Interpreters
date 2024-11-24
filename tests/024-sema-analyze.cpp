#include <networkprotocoldsl/lexer/token.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>
#include <networkprotocoldsl/parser/parse.hpp>
#include <networkprotocoldsl/sema/analyze.hpp>

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

  auto maybe_protocol = sema::analyze(protocol_description);
  ASSERT_TRUE(maybe_protocol.has_value());
  auto protocol = maybe_protocol.value();
  ASSERT_TRUE(protocol->client);
  ASSERT_TRUE(protocol->server);
}
