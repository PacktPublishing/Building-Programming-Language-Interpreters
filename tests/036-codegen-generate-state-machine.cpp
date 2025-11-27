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

TEST_F(GenerateStateMachineTest, InitialStateIsOpen) {
  auto result = generate_state_machine(*ctx_, *info_);

  // The initial state should be Open, not the alphabetically first state
  EXPECT_TRUE(result.header.find("current_state_ = State::Open") != std::string::npos)
      << "Initial state should be State::Open, not alphabetically first state. Header:\n"
      << result.header.substr(0, 2000);
}

TEST_F(GenerateStateMachineTest, ClientStateMachineHasParserMembers) {
  auto result = generate_state_machine(*ctx_, *info_);

  // Client state machine should have parser members for parsing server responses
  // Find the ClientStateMachine class section
  auto client_pos = result.header.find("class ClientStateMachine");
  ASSERT_NE(client_pos, std::string::npos) << "Should have ClientStateMachine";

  // After ClientStateMachine, before ServerStateMachine (or end)
  auto server_pos = result.header.find("class ServerStateMachine");
  std::string client_section = result.header.substr(client_pos, 
      server_pos != std::string::npos ? server_pos - client_pos : std::string::npos);

  // Should have parser members (for parsing server responses)
  EXPECT_TRUE(client_section.find("_parser_") != std::string::npos)
      << "ClientStateMachine should have parser members";
}

TEST_F(GenerateStateMachineTest, ServerStateMachineHasParserMembers) {
  auto result = generate_state_machine(*ctx_, *info_);

  // Server state machine should have parser members for parsing client commands
  auto server_pos = result.header.find("class ServerStateMachine");
  ASSERT_NE(server_pos, std::string::npos) << "Should have ServerStateMachine";

  std::string server_section = result.header.substr(server_pos);

  // Should have parser members (for parsing client commands)
  EXPECT_TRUE(server_section.find("_parser_") != std::string::npos)
      << "ServerStateMachine should have parser members";
}

// SMTP-specific tests using the real SMTP protocol definition
class GenerateStateMachineSMTPTest : public ::testing::Test {
protected:
  std::shared_ptr<const sema::ast::Protocol> protocol_;
  std::unique_ptr<OutputContext> ctx_;
  std::unique_ptr<ProtocolInfo> info_;

  void SetUp() override {
    std::string test_file =
        std::string(SMTP_SOURCE_DIR) + "/smtp.networkprotocoldsl";
    std::ifstream file(test_file);
    ASSERT_TRUE(file.is_open()) << "Could not open SMTP file: " << test_file;
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    auto maybe_tokens = lexer::tokenize(content);
    ASSERT_TRUE(maybe_tokens.has_value()) << "Failed to tokenize SMTP";

    auto result = parser::parse(maybe_tokens.value());
    ASSERT_TRUE(result.has_value()) << "Failed to parse SMTP";

    auto maybe_protocol = sema::analyze(result.value());
    ASSERT_TRUE(maybe_protocol.has_value()) << "Failed semantic analysis";
    protocol_ = maybe_protocol.value();

    ctx_ = std::make_unique<OutputContext>("smtp::generated");
    info_ = std::make_unique<ProtocolInfo>(protocol_);
  }
};

TEST_F(GenerateStateMachineSMTPTest, GeneratesValidCode) {
  auto result = generate_state_machine(*ctx_, *info_);

  EXPECT_FALSE(result.header.empty());
  EXPECT_FALSE(result.source.empty());
  EXPECT_TRUE(result.errors.empty())
      << "Errors: " << (result.errors.empty() ? "" : result.errors[0]);
}

TEST_F(GenerateStateMachineSMTPTest, InitialStateIsOpen) {
  auto result = generate_state_machine(*ctx_, *info_);

  // SMTP state machine should start at Open state
  EXPECT_TRUE(result.header.find("current_state_ = State::Open") != std::string::npos)
      << "Initial state should be State::Open for SMTP protocol";
}

TEST_F(GenerateStateMachineSMTPTest, HasClientSendEHLOStateCase) {
  auto result = generate_state_machine(*ctx_, *info_);

  // Server should have a case for ClientSendEHLO state to parse EHLO commands
  EXPECT_TRUE(result.source.find("case State::ClientSendEHLO:") != std::string::npos)
      << "ServerStateMachine should handle ClientSendEHLO state";
}

TEST_F(GenerateStateMachineSMTPTest, HasSMTPEHLOCommandParser) {
  auto result = generate_state_machine(*ctx_, *info_);

  // Server should have an EHLO command parser member
  EXPECT_TRUE(result.header.find("SMTPEHLOCommand") != std::string::npos)
      << "Should have SMTPEHLOCommand parser for parsing client EHLO commands";
}

TEST_F(GenerateStateMachineSMTPTest, ServerCanParseEHLOInClientSendEHLOState) {
  auto result = generate_state_machine(*ctx_, *info_);

  // Find the ClientSendEHLO case in the source
  auto case_pos = result.source.find("case State::ClientSendEHLO:");
  ASSERT_NE(case_pos, std::string::npos) << "Should have ClientSendEHLO case";

  // Find the next case statement to delimit the block
  auto next_case = result.source.find("case State::", case_pos + 10);
  std::string ehlo_case_block = result.source.substr(case_pos,
      next_case != std::string::npos ? next_case - case_pos : 500);

  // The block should reference EHLO parsing
  EXPECT_TRUE(ehlo_case_block.find("SMTPEHLOCommand") != std::string::npos)
      << "ClientSendEHLO case should parse SMTPEHLOCommand. Block was:\n"
      << ehlo_case_block;
}

TEST_F(GenerateStateMachineSMTPTest, SendMethodsTransitionToNextState) {
  auto result = generate_state_machine(*ctx_, *info_);

  // send_SMTPServerGreeting should transition from Open to ClientSendEHLO
  auto send_pos = result.source.find("ServerStateMachine::send_SMTPServerGreeting");
  ASSERT_NE(send_pos, std::string::npos) << "Should have send_SMTPServerGreeting method";

  // Find the function body (up to next function)
  auto func_end = result.source.find("\n}", send_pos);
  std::string send_func = result.source.substr(send_pos,
      func_end != std::string::npos ? func_end - send_pos + 2 : 500);

  // The send function should transition to ClientSendEHLO state
  EXPECT_TRUE(send_func.find("current_state_ = State::ClientSendEHLO") != std::string::npos)
      << "send_SMTPServerGreeting should transition to ClientSendEHLO. Function:\n"
      << send_func;
}

TEST_F(GenerateStateMachineSMTPTest, BytesWrittenManagesBuffer) {
  auto result = generate_state_machine(*ctx_, *info_);

  // bytes_written should manage the output buffer
  EXPECT_TRUE(result.source.find("bytes_written") != std::string::npos)
      << "Should have bytes_written method";

  // The function should clear or erase from buffer
  auto bw_pos = result.source.find("ServerStateMachine::bytes_written");
  ASSERT_NE(bw_pos, std::string::npos) << "Should have ServerStateMachine::bytes_written";

  auto func_end = result.source.find("\n}", bw_pos);
  std::string bw_func = result.source.substr(bw_pos,
      func_end != std::string::npos ? func_end - bw_pos + 2 : 500);

  // Should manage the output buffer
  EXPECT_TRUE(bw_func.find("output_buffer_") != std::string::npos)
      << "bytes_written should manage output buffer. Function:\n" << bw_func;
}

TEST_F(GenerateStateMachineSMTPTest, OnBytesReceivedHasClientSendEHLOCase) {
  auto result = generate_state_machine(*ctx_, *info_);

  // Find ServerStateMachine::on_bytes_received
  auto func_pos = result.source.find("ServerStateMachine::on_bytes_received");
  ASSERT_NE(func_pos, std::string::npos)
      << "Should have ServerStateMachine::on_bytes_received";

  // Find the switch statement within this function
  auto switch_pos = result.source.find("switch (current_state_)", func_pos);
  ASSERT_NE(switch_pos, std::string::npos)
      << "on_bytes_received should have switch on current_state_";

  // Find the ClientSendEHLO case after this switch
  auto ehlo_case = result.source.find("case State::ClientSendEHLO:", switch_pos);
  ASSERT_NE(ehlo_case, std::string::npos)
      << "ServerStateMachine::on_bytes_received should have ClientSendEHLO case";

  // Verify it's within the ServerStateMachine function (not ClientStateMachine)
  // by checking it comes after the ServerStateMachine function start
  EXPECT_GT(ehlo_case, func_pos)
      << "ClientSendEHLO case should be in ServerStateMachine::on_bytes_received";
}

TEST_F(GenerateStateMachineSMTPTest, ClientSendEHLOCaseCallsEHLOParser) {
  auto result = generate_state_machine(*ctx_, *info_);

  // Find ServerStateMachine::on_bytes_received
  auto func_pos = result.source.find("ServerStateMachine::on_bytes_received");
  ASSERT_NE(func_pos, std::string::npos);

  // Find the ClientSendEHLO case
  auto ehlo_case = result.source.find("case State::ClientSendEHLO:", func_pos);
  ASSERT_NE(ehlo_case, std::string::npos);

  // Find the next case or default
  auto next_case = result.source.find("case State::", ehlo_case + 20);
  auto default_case = result.source.find("default:", ehlo_case + 20);
  size_t case_end = std::min(
      next_case != std::string::npos ? next_case : result.source.size(),
      default_case != std::string::npos ? default_case : result.source.size());

  std::string ehlo_block = result.source.substr(ehlo_case, case_end - ehlo_case);

  // The case should call EHLO parser
  EXPECT_TRUE(ehlo_block.find("SMTPEHLOCommand_parser_") != std::string::npos)
      << "ClientSendEHLO case should use SMTPEHLOCommand_parser_. Block:\n"
      << ehlo_block;

  // Should call parse method
  EXPECT_TRUE(ehlo_block.find(".parse(") != std::string::npos)
      << "Should call parse method. Block:\n" << ehlo_block;
}

TEST_F(GenerateStateMachineSMTPTest, ClientSendEHLOCaseSetsHasMessage) {
  auto result = generate_state_machine(*ctx_, *info_);

  auto func_pos = result.source.find("ServerStateMachine::on_bytes_received");
  ASSERT_NE(func_pos, std::string::npos);

  auto ehlo_case = result.source.find("case State::ClientSendEHLO:", func_pos);
  ASSERT_NE(ehlo_case, std::string::npos);

  auto next_case = result.source.find("case State::", ehlo_case + 20);
  auto default_case = result.source.find("default:", ehlo_case + 20);
  size_t case_end = std::min(
      next_case != std::string::npos ? next_case : result.source.size(),
      default_case != std::string::npos ? default_case : result.source.size());

  std::string ehlo_block = result.source.substr(ehlo_case, case_end - ehlo_case);

  // Should set has_message_ = true on successful parse
  EXPECT_TRUE(ehlo_block.find("has_message_ = true") != std::string::npos)
      << "Should set has_message_ = true. Block:\n" << ehlo_block;
}

TEST_F(GenerateStateMachineSMTPTest, ClientSendEHLOCaseUpdatesConsumed) {
  auto result = generate_state_machine(*ctx_, *info_);

  auto func_pos = result.source.find("ServerStateMachine::on_bytes_received");
  ASSERT_NE(func_pos, std::string::npos);

  auto ehlo_case = result.source.find("case State::ClientSendEHLO:", func_pos);
  ASSERT_NE(ehlo_case, std::string::npos);

  auto next_case = result.source.find("case State::", ehlo_case + 20);
  auto default_case = result.source.find("default:", ehlo_case + 20);
  size_t case_end = std::min(
      next_case != std::string::npos ? next_case : result.source.size(),
      default_case != std::string::npos ? default_case : result.source.size());

  std::string ehlo_block = result.source.substr(ehlo_case, case_end - ehlo_case);

  // Should update total_consumed from parse result
  EXPECT_TRUE(ehlo_block.find("total_consumed") != std::string::npos)
      << "Should update total_consumed. Block:\n" << ehlo_block;

  // Should get consumed from result
  EXPECT_TRUE(ehlo_block.find(".consumed") != std::string::npos)
      << "Should use result.consumed. Block:\n" << ehlo_block;
}

TEST_F(GenerateStateMachineSMTPTest, ClientSendEHLOCaseTransitionsToEHLOResponse) {
  auto result = generate_state_machine(*ctx_, *info_);

  auto func_pos = result.source.find("ServerStateMachine::on_bytes_received");
  ASSERT_NE(func_pos, std::string::npos);

  auto ehlo_case = result.source.find("case State::ClientSendEHLO:", func_pos);
  ASSERT_NE(ehlo_case, std::string::npos);

  auto next_case = result.source.find("case State::", ehlo_case + 20);
  auto default_case = result.source.find("default:", ehlo_case + 20);
  size_t case_end = std::min(
      next_case != std::string::npos ? next_case : result.source.size(),
      default_case != std::string::npos ? default_case : result.source.size());

  std::string ehlo_block = result.source.substr(ehlo_case, case_end - ehlo_case);

  // Should transition to AwaitServerEHLOResponse after parsing EHLO
  EXPECT_TRUE(ehlo_block.find("State::AwaitServerEHLOResponse") != std::string::npos)
      << "Should transition to AwaitServerEHLOResponse. Block:\n" << ehlo_block;
}

TEST_F(GenerateStateMachineSMTPTest, OnBytesReceivedReturnsConsumed) {
  auto result = generate_state_machine(*ctx_, *info_);

  auto func_pos = result.source.find("ServerStateMachine::on_bytes_received");
  ASSERT_NE(func_pos, std::string::npos);

  // Find the return statement
  auto return_pos = result.source.find("return total_consumed", func_pos);
  EXPECT_NE(return_pos, std::string::npos)
      << "on_bytes_received should return total_consumed";
}

