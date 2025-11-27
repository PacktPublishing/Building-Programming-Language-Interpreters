#include <networkprotocoldsl/codegen/generate_states.hpp>
#include <networkprotocoldsl/codegen/outputcontext.hpp>
#include <networkprotocoldsl/codegen/protocolinfo.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>
#include <networkprotocoldsl/parser/parse.hpp>
#include <networkprotocoldsl/sema/analyze.hpp>

#include <fstream>
#include <gtest/gtest.h>

using namespace networkprotocoldsl;
using namespace networkprotocoldsl::codegen;

class GenerateStatesTest : public ::testing::Test {
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

TEST_F(GenerateStatesTest, GeneratesHeader) {
  auto result = generate_states(*ctx_, *info_);

  EXPECT_FALSE(result.header.empty());
  EXPECT_TRUE(result.errors.empty());
}

TEST_F(GenerateStatesTest, GeneratesSource) {
  auto result = generate_states(*ctx_, *info_);

  EXPECT_FALSE(result.source.empty());
}

TEST_F(GenerateStatesTest, HeaderHasStateEnum) {
  auto result = generate_states(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("enum class State") != std::string::npos);
}

TEST_F(GenerateStatesTest, HeaderHasExpectedStates) {
  auto result = generate_states(*ctx_, *info_);

  // Should have Open, Closed, AwaitResponse states
  EXPECT_TRUE(result.header.find("Open") != std::string::npos);
  EXPECT_TRUE(result.header.find("Closed") != std::string::npos);
  EXPECT_TRUE(result.header.find("AwaitResponse") != std::string::npos);
}

TEST_F(GenerateStatesTest, HeaderHasVariantTypes) {
  auto result = generate_states(*ctx_, *info_);

  // Should have variant types for inputs/outputs
  EXPECT_TRUE(result.header.find("std::variant") != std::string::npos);
}

TEST_F(GenerateStatesTest, HeaderIncludesDataTypes) {
  auto result = generate_states(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("#include \"data_types.hpp\"") !=
              std::string::npos);
}

TEST_F(GenerateStatesTest, HeaderHasGuard) {
  auto result = generate_states(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("#ifndef") != std::string::npos);
  EXPECT_TRUE(result.header.find("#define") != std::string::npos);
  EXPECT_TRUE(result.header.find("#endif") != std::string::npos);
}

TEST_F(GenerateStatesTest, HeaderHasNamespace) {
  auto result = generate_states(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("namespace test::http") != std::string::npos);
}

TEST_F(GenerateStatesTest, SourceIncludesHeader) {
  auto result = generate_states(*ctx_, *info_);

  EXPECT_TRUE(result.source.find("#include \"states.hpp\"") !=
              std::string::npos);
}

TEST_F(GenerateStatesTest, HasOutputVariantTypes) {
  auto result = generate_states(*ctx_, *info_);

  // Should have Output variant types for states that have outgoing transitions
  EXPECT_TRUE(result.header.find("Output = std::variant") != std::string::npos);
}

TEST_F(GenerateStatesTest, HasInputVariantTypes) {
  auto result = generate_states(*ctx_, *info_);

  // Should have Input variant types for states that have incoming transitions
  EXPECT_TRUE(result.header.find("Input = std::variant") != std::string::npos);
}

TEST_F(GenerateStatesTest, VariantsReferenceDataTypes) {
  auto result = generate_states(*ctx_, *info_);

  // Variants should reference *Data types
  EXPECT_TRUE(result.header.find("Data") != std::string::npos);
}

TEST_F(GenerateStatesTest, ClosedInputExists) {
  auto result = generate_states(*ctx_, *info_);

  // Closed state should have an Input variant for the final message received
  EXPECT_TRUE(result.header.find("ClosedInput") != std::string::npos)
      << "Closed state should have an Input type for the message that closes the connection";
}

TEST_F(GenerateStatesTest, StateEnumIsEnumClass) {
  auto result = generate_states(*ctx_, *info_);

  // Should be a scoped enum
  EXPECT_TRUE(result.header.find("enum class State") != std::string::npos);
}

TEST_F(GenerateStatesTest, ClosingBracesMatch) {
  auto result = generate_states(*ctx_, *info_);

  // Count opening and closing braces - they should match
  int open_braces = 0;
  int close_braces = 0;
  for (char c : result.header) {
    if (c == '{') open_braces++;
    if (c == '}') close_braces++;
  }
  EXPECT_EQ(open_braces, close_braces) << "Braces should be balanced in header";
}

TEST_F(GenerateStatesTest, NoErrorsReported) {
  auto result = generate_states(*ctx_, *info_);

  EXPECT_TRUE(result.errors.empty())
      << "Should not have errors, but got: "
      << (result.errors.empty() ? "" : result.errors[0]);
}

TEST_F(GenerateStatesTest, StateEnumEndsWithComma) {
  auto result = generate_states(*ctx_, *info_);

  // Each state enum value should be followed by comma
  // Find the enum block and check format
  auto enum_pos = result.header.find("enum class State");
  EXPECT_TRUE(enum_pos != std::string::npos);
  
  // After enum class State { there should be state names
  auto open_brace = result.header.find("{", enum_pos);
  auto close_brace = result.header.find("}", open_brace);
  
  std::string enum_body = result.header.substr(open_brace + 1, close_brace - open_brace - 1);
  
  // Should contain at least "Open," and "Closed,"
  EXPECT_TRUE(enum_body.find("Open,") != std::string::npos);
  EXPECT_TRUE(enum_body.find("Closed,") != std::string::npos);
}
