#include <networkprotocoldsl/codegen/generate_data_types.hpp>
#include <networkprotocoldsl/codegen/outputcontext.hpp>
#include <networkprotocoldsl/codegen/protocolinfo.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>
#include <networkprotocoldsl/parser/parse.hpp>
#include <networkprotocoldsl/sema/analyze.hpp>

#include <fstream>
#include <gtest/gtest.h>

using namespace networkprotocoldsl;
using namespace networkprotocoldsl::codegen;

class GenerateDataTypesTest : public ::testing::Test {
protected:
  std::shared_ptr<const sema::ast::Protocol> protocol_;
  std::unique_ptr<OutputContext> ctx_;
  std::unique_ptr<ProtocolInfo> info_;

  void SetUp() override {
    std::string test_file =
        std::string(TEST_DATA_DIR) + "/023-source-code-http-client-server.txt";
    std::ifstream file(test_file);
    ASSERT_TRUE(file.is_open());
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    auto maybe_tokens = lexer::tokenize(content);
    ASSERT_TRUE(maybe_tokens.has_value());

    auto result = parser::parse(maybe_tokens.value());
    ASSERT_TRUE(result.has_value());

    auto maybe_protocol = sema::analyze(result.value());
    ASSERT_TRUE(maybe_protocol.has_value());
    protocol_ = maybe_protocol.value();

    ctx_ = std::make_unique<OutputContext>("test::http");
    info_ = std::make_unique<ProtocolInfo>(protocol_);
  }
};

TEST_F(GenerateDataTypesTest, GeneratesHeader) {
  auto result = generate_data_types(*ctx_, *info_);

  EXPECT_FALSE(result.header.empty());
  EXPECT_TRUE(result.errors.empty());
}

TEST_F(GenerateDataTypesTest, GeneratesSource) {
  auto result = generate_data_types(*ctx_, *info_);

  EXPECT_FALSE(result.source.empty());
}

TEST_F(GenerateDataTypesTest, HeaderHasGuard) {
  auto result = generate_data_types(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("#ifndef") != std::string::npos);
  EXPECT_TRUE(result.header.find("#define") != std::string::npos);
  EXPECT_TRUE(result.header.find("#endif") != std::string::npos);
}

TEST_F(GenerateDataTypesTest, HeaderHasNamespace) {
  auto result = generate_data_types(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("namespace test::http") != std::string::npos);
}

TEST_F(GenerateDataTypesTest, HeaderHasDataStructs) {
  auto result = generate_data_types(*ctx_, *info_);

  // Should contain struct definitions for messages
  EXPECT_TRUE(result.header.find("struct") != std::string::npos);
  EXPECT_TRUE(result.header.find("Data") != std::string::npos);
}

TEST_F(GenerateDataTypesTest, HeaderHasRequiredIncludes) {
  auto result = generate_data_types(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("#include <cstdint>") != std::string::npos);
  EXPECT_TRUE(result.header.find("#include <string>") != std::string::npos);
  EXPECT_TRUE(result.header.find("#include <vector>") != std::string::npos);
}

TEST_F(GenerateDataTypesTest, SourceIncludesHeader) {
  auto result = generate_data_types(*ctx_, *info_);

  EXPECT_TRUE(result.source.find("#include \"data_types.hpp\"") !=
              std::string::npos);
}

TEST_F(GenerateDataTypesTest, SourceHasNamespace) {
  auto result = generate_data_types(*ctx_, *info_);

  EXPECT_TRUE(result.source.find("namespace test::http") != std::string::npos);
}

TEST_F(GenerateDataTypesTest, GeneratesHTTPRequestData) {
  auto result = generate_data_types(*ctx_, *info_);

  // Should have HTTPRequestData struct
  EXPECT_TRUE(result.header.find("HTTPRequestData") != std::string::npos);
}

TEST_F(GenerateDataTypesTest, GeneratesHTTPResponseData) {
  auto result = generate_data_types(*ctx_, *info_);

  // Should have HTTPResponseData struct
  EXPECT_TRUE(result.header.find("HTTPResponseData") != std::string::npos);
}

TEST_F(GenerateDataTypesTest, HasStringFields) {
  auto result = generate_data_types(*ctx_, *info_);

  // HTTP Request has verb, request_target which are strings
  EXPECT_TRUE(result.header.find("std::string") != std::string::npos);
}

TEST_F(GenerateDataTypesTest, HasIntFields) {
  auto result = generate_data_types(*ctx_, *info_);

  // HTTP has major_version, minor_version which are integers
  EXPECT_TRUE(result.header.find("uint8_t") != std::string::npos);
}

TEST_F(GenerateDataTypesTest, HasVectorFields) {
  auto result = generate_data_types(*ctx_, *info_);

  // HTTP has headers which is an array
  EXPECT_TRUE(result.header.find("std::vector") != std::string::npos);
}

TEST_F(GenerateDataTypesTest, GeneratesEmptyStructForDatalessMessage) {
  auto result = generate_data_types(*ctx_, *info_);

  // "Client Closes Connection" has no data
  EXPECT_TRUE(result.header.find("ClientClosesConnectionData") != std::string::npos);
}

TEST_F(GenerateDataTypesTest, ClosingBracesMatch) {
  auto result = generate_data_types(*ctx_, *info_);

  // Count opening and closing braces - they should match
  int open_braces = 0;
  int close_braces = 0;
  for (char c : result.header) {
    if (c == '{') open_braces++;
    if (c == '}') close_braces++;
  }
  EXPECT_EQ(open_braces, close_braces) << "Braces should be balanced in header";
}

TEST_F(GenerateDataTypesTest, NoErrorsReported) {
  auto result = generate_data_types(*ctx_, *info_);

  EXPECT_TRUE(result.errors.empty()) 
      << "Should not have errors, but got: " 
      << (result.errors.empty() ? "" : result.errors[0]);
}
