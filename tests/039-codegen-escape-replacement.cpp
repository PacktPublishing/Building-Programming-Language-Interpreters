#include <networkprotocoldsl/codegen/generate_parser.hpp>
#include <networkprotocoldsl/codegen/generate_serializer.hpp>
#include <networkprotocoldsl/codegen/outputcontext.hpp>
#include <networkprotocoldsl/codegen/protocolinfo.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>
#include <networkprotocoldsl/parser/parse.hpp>
#include <networkprotocoldsl/sema/analyze.hpp>

#include <fstream>
#include <gtest/gtest.h>

using namespace networkprotocoldsl;
using namespace networkprotocoldsl::codegen;

class EscapeReplacementCodegenTest : public ::testing::Test {
protected:
  std::shared_ptr<const sema::ast::Protocol> protocol_;
  std::unique_ptr<OutputContext> ctx_;
  std::unique_ptr<ProtocolInfo> info_;

  void SetUp() override {
    std::string test_file =
        std::string(TEST_DATA_DIR) + "/038-http-with-continuation.txt";
    std::ifstream file(test_file);
    ASSERT_TRUE(file.is_open()) << "Could not open " << test_file;
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    auto maybe_tokens = lexer::tokenize(content);
    ASSERT_TRUE(maybe_tokens.has_value()) << "Tokenization failed";

    auto result = parser::parse(maybe_tokens.value());
    ASSERT_TRUE(result.has_value()) << "Parsing failed";

    auto maybe_protocol = sema::analyze(result.value());
    ASSERT_TRUE(maybe_protocol.has_value()) << "Semantic analysis failed";
    protocol_ = maybe_protocol.value();

    ctx_ = std::make_unique<OutputContext>("test::http_escape");
    info_ = std::make_unique<ProtocolInfo>(protocol_);
  }
};

// Test that parser code is generated with escape handling
TEST_F(EscapeReplacementCodegenTest, ParserGeneratesEscapeHandling) {
  auto result = generate_parser(*ctx_, *info_);

  EXPECT_TRUE(result.errors.empty()) << "Parser generation had errors";
  EXPECT_FALSE(result.source.empty()) << "Parser source is empty";

  // The generated parser should contain escape sequence handling
  // Looking for the escape_sequence variable or the replacement logic
  EXPECT_TRUE(result.source.find("escape_sequence") != std::string::npos ||
              result.source.find("\\r\\n ") != std::string::npos)
      << "Parser source should contain escape sequence handling";
}

// Test that parser handles the specific HTTP continuation escape
TEST_F(EscapeReplacementCodegenTest, ParserHasHTTPContinuationEscape) {
  auto result = generate_parser(*ctx_, *info_);

  // Should contain the escape sequence "\r\n " (CRLF + space)
  // In C++ string literals, this would appear as "\\r\\n "
  bool has_crlf_space = result.source.find("\\r\\n ") != std::string::npos;
  
  EXPECT_TRUE(has_crlf_space)
      << "Parser should handle CRLF+space escape sequence for HTTP continuation";
}

// Test that parser has the escape character replacement
TEST_F(EscapeReplacementCodegenTest, ParserHasEscapeCharReplacement) {
  auto result = generate_parser(*ctx_, *info_);

  // The parser should insert "\n" when it finds the escape sequence
  // Look for escape_char or the newline replacement
  bool has_escape_char = result.source.find("escape_char") != std::string::npos;
  bool has_newline_replacement = result.source.find("\"\\n\"") != std::string::npos;

  EXPECT_TRUE(has_escape_char || has_newline_replacement)
      << "Parser should contain escape character replacement logic";
}

// Test that serializer code is generated with escape handling
TEST_F(EscapeReplacementCodegenTest, SerializerGeneratesEscapeHandling) {
  auto result = generate_serializer(*ctx_, *info_);

  EXPECT_TRUE(result.errors.empty()) << "Serializer generation had errors";
  EXPECT_FALSE(result.source.empty()) << "Serializer source is empty";

  // The generated serializer should contain escape replacement logic
  // It should replace "\n" with "\r\n " when serializing
  EXPECT_TRUE(result.source.find("escape") != std::string::npos ||
              result.source.find("replace") != std::string::npos ||
              result.source.find("find") != std::string::npos)
      << "Serializer source should contain escape replacement logic";
}

// Test that serializer has the specific HTTP continuation escape
TEST_F(EscapeReplacementCodegenTest, SerializerHasHTTPContinuationEscape) {
  auto result = generate_serializer(*ctx_, *info_);

  // Should contain logic to replace "\n" with "\r\n "
  bool has_crlf_space = result.source.find("\\r\\n ") != std::string::npos;

  EXPECT_TRUE(has_crlf_space)
      << "Serializer should output CRLF+space for HTTP continuation";
}

// Test that serializer has newline detection
TEST_F(EscapeReplacementCodegenTest, SerializerHasNewlineDetection) {
  auto result = generate_serializer(*ctx_, *info_);

  // Should look for "\n" in the value to escape it
  bool has_newline_search = result.source.find("\"\\n\"") != std::string::npos;

  EXPECT_TRUE(has_newline_search)
      << "Serializer should search for newlines to escape";
}

// Test that both parser and serializer are generated without errors
TEST_F(EscapeReplacementCodegenTest, BothGenerateWithoutErrors) {
  auto parser_result = generate_parser(*ctx_, *info_);
  auto serializer_result = generate_serializer(*ctx_, *info_);

  EXPECT_TRUE(parser_result.errors.empty())
      << "Parser should generate without errors";
  EXPECT_TRUE(serializer_result.errors.empty())
      << "Serializer should generate without errors";
}

// Test that headers are generated correctly
TEST_F(EscapeReplacementCodegenTest, HeadersAreGenerated) {
  auto parser_result = generate_parser(*ctx_, *info_);
  auto serializer_result = generate_serializer(*ctx_, *info_);

  EXPECT_FALSE(parser_result.header.empty())
      << "Parser header should not be empty";
  EXPECT_FALSE(serializer_result.header.empty())
      << "Serializer header should not be empty";

  // Headers should have proper guards
  EXPECT_TRUE(parser_result.header.find("#ifndef") != std::string::npos);
  EXPECT_TRUE(serializer_result.header.find("#ifndef") != std::string::npos);
}

// Test that the protocol info correctly identifies escape sequences
TEST_F(EscapeReplacementCodegenTest, ProtocolInfoHasEscapeInfo) {
  // Check that the sema correctly parsed the escape info
  // The protocol should have messages with escape-enabled fields
  ASSERT_TRUE(protocol_ != nullptr);
  
  bool found_escape = false;
  
  // Helper lambda to check an agent's transitions for escape info
  auto check_agent = [&](const std::shared_ptr<const sema::ast::Agent>& agent) {
    if (!agent) return;
    for (const auto& [state_name, state] : agent->states) {
      for (const auto& [msg_name, trans_pair] : state->transitions) {
        const auto& [transition, next_state] = trans_pair;
        
        std::visit([&](const auto& trans) {
          for (const auto& action : trans->actions) {
            if (auto* read_action = std::get_if<std::shared_ptr<const sema::ast::action::ReadOctetsUntilTerminator>>(&action)) {
              if ((*read_action)->escape.has_value()) {
                found_escape = true;
                EXPECT_EQ("\n", (*read_action)->escape->character);
                EXPECT_EQ("\r\n ", (*read_action)->escape->sequence);
              }
            }
            if (auto* write_action = std::get_if<std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(&action)) {
              if ((*write_action)->escape.has_value()) {
                found_escape = true;
                EXPECT_EQ("\n", (*write_action)->escape->character);
                EXPECT_EQ("\r\n ", (*write_action)->escape->sequence);
              }
            }
          }
        }, transition);
      }
    }
  };
  
  check_agent(protocol_->client);
  check_agent(protocol_->server);
  
  EXPECT_TRUE(found_escape) << "Protocol should have at least one field with escape info";
}
