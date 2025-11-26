#include <networkprotocoldsl/codegen/generate_state_machine.hpp>
#include <networkprotocoldsl/codegen/outputcontext.hpp>
#include <networkprotocoldsl/codegen/protocolinfo.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>
#include <networkprotocoldsl/parser/parse.hpp>
#include <networkprotocoldsl/sema/analyze.hpp>

#include <fstream>
#include <gtest/gtest.h>

using namespace networkprotocoldsl;
using namespace networkprotocoldsl::codegen;

class GenerateStateMachineTest : public ::testing::Test {
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

TEST_F(GenerateStateMachineTest, GeneratesHeader) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_FALSE(result.header.empty());
  EXPECT_TRUE(result.errors.empty());
}

TEST_F(GenerateStateMachineTest, GeneratesSource) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_FALSE(result.source.empty());
}

TEST_F(GenerateStateMachineTest, HeaderHasGuard) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("#ifndef") != std::string::npos);
  EXPECT_TRUE(result.header.find("#define") != std::string::npos);
  EXPECT_TRUE(result.header.find("#endif") != std::string::npos);
}

TEST_F(GenerateStateMachineTest, HeaderHasNamespace) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("namespace test::http") != std::string::npos);
}

TEST_F(GenerateStateMachineTest, HeaderIncludesParser) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("#include \"parser.hpp\"") !=
              std::string::npos);
}

TEST_F(GenerateStateMachineTest, HeaderIncludesSerializer) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("#include \"serializer.hpp\"") !=
              std::string::npos);
}

TEST_F(GenerateStateMachineTest, HeaderIncludesStates) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("#include \"states.hpp\"") !=
              std::string::npos);
}

TEST_F(GenerateStateMachineTest, SourceIncludesHeader) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_TRUE(result.source.find("#include \"state_machine.hpp\"") !=
              std::string::npos);
}

TEST_F(GenerateStateMachineTest, SourceHasNamespace) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_TRUE(result.source.find("namespace test::http") != std::string::npos);
}

TEST_F(GenerateStateMachineTest, HeaderHasOptionalInclude) {
  auto result = generate_state_machine(*ctx_, *info_);

  // State machine uses std::optional for pending message storage
  EXPECT_TRUE(result.header.find("#include <optional>") != std::string::npos);
}

TEST_F(GenerateStateMachineTest, HeaderDoesNotHaveUnusedIncludes) {
  auto result = generate_state_machine(*ctx_, *info_);

  // These includes were removed as they're not used
  EXPECT_TRUE(result.header.find("#include <mutex>") == std::string::npos)
      << "Should not include unused <mutex>";
  EXPECT_TRUE(result.header.find("#include <atomic>") == std::string::npos)
      << "Should not include unused <atomic>";
  EXPECT_TRUE(result.header.find("#include <functional>") == std::string::npos)
      << "Should not include unused <functional>";
  EXPECT_TRUE(result.header.find("#include <queue>") == std::string::npos)
      << "Should not include unused <queue>";
  EXPECT_TRUE(result.header.find("#include <map>") == std::string::npos)
      << "Should not include unused <map>";
}

TEST_F(GenerateStateMachineTest, ClosingBracesMatch) {
  auto result = generate_state_machine(*ctx_, *info_);

  // Count opening and closing braces - they should match
  int open_braces = 0;
  int close_braces = 0;
  for (char c : result.header) {
    if (c == '{') open_braces++;
    if (c == '}') close_braces++;
  }
  EXPECT_EQ(open_braces, close_braces) << "Braces should be balanced in header";
}

TEST_F(GenerateStateMachineTest, NoErrorsReported) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_TRUE(result.errors.empty())
      << "Should not have errors, but got: "
      << (result.errors.empty() ? "" : result.errors[0]);
}

TEST_F(GenerateStateMachineTest, HeaderHasClientStateMachine) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("class ClientStateMachine") != std::string::npos)
      << "Should have ClientStateMachine class";
}

TEST_F(GenerateStateMachineTest, HeaderHasServerStateMachine) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("class ServerStateMachine") != std::string::npos)
      << "Should have ServerStateMachine class";
}

TEST_F(GenerateStateMachineTest, HeaderHasOnBytesReceivedMethod) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("on_bytes_received") != std::string::npos)
      << "Should have on_bytes_received method";
}

TEST_F(GenerateStateMachineTest, HeaderHasPendingOutputMethod) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("pending_output") != std::string::npos)
      << "Should have pending_output method";
}

TEST_F(GenerateStateMachineTest, HeaderHasSendMethods) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("void send_") != std::string::npos)
      << "Should have send_ methods for outgoing messages";
}

TEST_F(GenerateStateMachineTest, HeaderHasCurrentStateMethod) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("current_state()") != std::string::npos)
      << "Should have current_state method";
}

TEST_F(GenerateStateMachineTest, HeaderHasStateSpecificTakeMessageMethods) {
  auto result = generate_state_machine(*ctx_, *info_);

  // Should have state-specific take_*_message methods instead of generic take_message
  EXPECT_TRUE(result.header.find("take_") != std::string::npos &&
              result.header.find("_message()") != std::string::npos)
      << "Should have state-specific take_*_message methods";

  // Should have message_state() method to know which take method to call
  EXPECT_TRUE(result.header.find("message_state()") != std::string::npos)
      << "Should have message_state method";
}

TEST_F(GenerateStateMachineTest, HeaderHasIsClosedMethod) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("is_closed()") != std::string::npos)
      << "Should have is_closed method";
}

TEST_F(GenerateStateMachineTest, SourceHasStateSwitch) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_TRUE(result.source.find("switch (current_state_)") != std::string::npos)
      << "Should have state-based dispatch";
}

TEST_F(GenerateStateMachineTest, SourceBalancedBraces) {
  auto result = generate_state_machine(*ctx_, *info_);

  int open_braces = 0;
  int close_braces = 0;
  for (char c : result.source) {
    if (c == '{') open_braces++;
    if (c == '}') close_braces++;
  }
  EXPECT_EQ(open_braces, close_braces) << "Braces should be balanced in source";
}

TEST_F(GenerateStateMachineTest, HeaderIncludesDataTypes) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("#include \"data_types.hpp\"") != std::string::npos)
      << "Should include data_types.hpp";
}

TEST_F(GenerateStateMachineTest, HeaderHasAutoGeneratedComment) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("auto-generated") != std::string::npos)
      << "Header should have auto-generated comment";
}

TEST_F(GenerateStateMachineTest, SourceHasAutoGeneratedComment) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_TRUE(result.source.find("auto-generated") != std::string::npos)
      << "Source should have auto-generated comment";
}

TEST_F(GenerateStateMachineTest, HeaderHasOnEofMethod) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("void on_eof()") != std::string::npos)
      << "Should have on_eof method for EOF handling";
}

TEST_F(GenerateStateMachineTest, SourceHasOnEofImplementation) {
  auto result = generate_state_machine(*ctx_, *info_);

  // on_eof should notify parsers
  EXPECT_TRUE(result.source.find("::on_eof()") != std::string::npos)
      << "Should have on_eof implementation";
  EXPECT_TRUE(result.source.find("_parser_.on_eof()") != std::string::npos)
      << "on_eof should call parser's on_eof";
}
