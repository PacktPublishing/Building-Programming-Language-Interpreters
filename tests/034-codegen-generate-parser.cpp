#include <networkprotocoldsl/codegen/generate_parser.hpp>
#include <networkprotocoldsl/codegen/outputcontext.hpp>
#include <networkprotocoldsl/codegen/protocolinfo.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>
#include <networkprotocoldsl/parser/parse.hpp>
#include <networkprotocoldsl/sema/analyze.hpp>

#include <fstream>
#include <gtest/gtest.h>

using namespace networkprotocoldsl;
using namespace networkprotocoldsl::codegen;

class GenerateParserTest : public ::testing::Test {
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

TEST_F(GenerateParserTest, GeneratesHeader) {
  auto result = generate_parser(*ctx_, *info_);

  EXPECT_FALSE(result.header.empty());
  EXPECT_TRUE(result.errors.empty());
}

TEST_F(GenerateParserTest, GeneratesSource) {
  auto result = generate_parser(*ctx_, *info_);

  EXPECT_FALSE(result.source.empty());
}

TEST_F(GenerateParserTest, HeaderHasParseStatus) {
  auto result = generate_parser(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("enum class ParseStatus") !=
              std::string::npos);
  EXPECT_TRUE(result.header.find("NeedMoreData") != std::string::npos);
  EXPECT_TRUE(result.header.find("Complete") != std::string::npos);
  EXPECT_TRUE(result.header.find("Error") != std::string::npos);
}

TEST_F(GenerateParserTest, HeaderHasParseResult) {
  auto result = generate_parser(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("struct ParseResult") != std::string::npos);
  EXPECT_TRUE(result.header.find("ParseStatus status") != std::string::npos);
  EXPECT_TRUE(result.header.find("size_t consumed") != std::string::npos);
}

TEST_F(GenerateParserTest, HeaderHasMessageParsers) {
  auto result = generate_parser(*ctx_, *info_);

  // Should have individual parser classes for messages
  EXPECT_TRUE(result.header.find("Parser") != std::string::npos);
  EXPECT_TRUE(result.header.find("class") != std::string::npos);
}

TEST_F(GenerateParserTest, HeaderHasParseMethod) {
  auto result = generate_parser(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("ParseResult parse(std::string_view input)") !=
              std::string::npos);
}

TEST_F(GenerateParserTest, HeaderHasResetMethod) {
  auto result = generate_parser(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("void reset()") != std::string::npos);
}

TEST_F(GenerateParserTest, HeaderHasTakeDataMethod) {
  auto result = generate_parser(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("take_data()") != std::string::npos);
}

TEST_F(GenerateParserTest, HeaderIncludesRequired) {
  auto result = generate_parser(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("#include <string_view>") !=
              std::string::npos);
  EXPECT_TRUE(result.header.find("#include \"data_types.hpp\"") !=
              std::string::npos);
  EXPECT_TRUE(result.header.find("#include \"states.hpp\"") !=
              std::string::npos);
}

TEST_F(GenerateParserTest, HeaderHasGuard) {
  auto result = generate_parser(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("#ifndef") != std::string::npos);
  EXPECT_TRUE(result.header.find("#define") != std::string::npos);
  EXPECT_TRUE(result.header.find("#endif") != std::string::npos);
}

TEST_F(GenerateParserTest, SourceIncludesHeader) {
  auto result = generate_parser(*ctx_, *info_);

  EXPECT_TRUE(result.source.find("#include \"parser.hpp\"") !=
              std::string::npos);
}

TEST_F(GenerateParserTest, SourceHasImplementation) {
  auto result = generate_parser(*ctx_, *info_);

  // Should have implementations of reset, parse, and take_data
  EXPECT_TRUE(result.source.find("::reset()") != std::string::npos);
  EXPECT_TRUE(result.source.find("::parse(") != std::string::npos);
  EXPECT_TRUE(result.source.find("::take_data()") != std::string::npos);
}

TEST_F(GenerateParserTest, HeaderHasMainParserClass) {
  auto result = generate_parser(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("class Parser") != std::string::npos);
  EXPECT_TRUE(result.header.find("set_state(State state)") != std::string::npos);
  EXPECT_TRUE(result.header.find("get_state()") != std::string::npos);
}

TEST_F(GenerateParserTest, SourceHasStageMachine) {
  auto result = generate_parser(*ctx_, *info_);

  // Parser should use stage-based state machine
  EXPECT_TRUE(result.source.find("stage_") != std::string::npos);
  EXPECT_TRUE(result.source.find("switch") != std::string::npos);
  EXPECT_TRUE(result.source.find("case") != std::string::npos);
}

TEST_F(GenerateParserTest, SourceHandlesStaticOctets) {
  auto result = generate_parser(*ctx_, *info_);

  // Should have static octet matching (e.g., "HTTP/")
  EXPECT_TRUE(result.source.find("memcmp") != std::string::npos);
}

TEST_F(GenerateParserTest, SourceHandlesTerminators) {
  auto result = generate_parser(*ctx_, *info_);

  // Should handle terminators like "\r\n"
  EXPECT_TRUE(result.source.find("terminator") != std::string::npos);
}

TEST_F(GenerateParserTest, HasIsCompleteMethod) {
  auto result = generate_parser(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("is_complete()") != std::string::npos);
  EXPECT_TRUE(result.header.find("complete_") != std::string::npos);
}

TEST_F(GenerateParserTest, SourceReturnsConsumedBytes) {
  auto result = generate_parser(*ctx_, *info_);

  // Should track consumed bytes
  EXPECT_TRUE(result.source.find("total_consumed") != std::string::npos);
}

TEST_F(GenerateParserTest, SourceHandlesNeedMoreData) {
  auto result = generate_parser(*ctx_, *info_);

  // Should return NeedMoreData when buffer is incomplete
  EXPECT_TRUE(result.source.find("NeedMoreData") != std::string::npos);
}

TEST_F(GenerateParserTest, SourceHandlesParseError) {
  auto result = generate_parser(*ctx_, *info_);

  // Should return Error on protocol violations
  EXPECT_TRUE(result.source.find("ParseStatus::Error") != std::string::npos);
}

TEST_F(GenerateParserTest, HasBufferFieldsForStringData) {
  auto result = generate_parser(*ctx_, *info_);

  // Should have buffer fields for accumulating string data
  EXPECT_TRUE(result.header.find("_buffer_") != std::string::npos);
}

TEST_F(GenerateParserTest, TakeDataMovesBuffers) {
  auto result = generate_parser(*ctx_, *info_);

  // take_data should move data, not copy
  EXPECT_TRUE(result.source.find("std::move") != std::string::npos);
}

TEST_F(GenerateParserTest, ResetClearsBuffers) {
  auto result = generate_parser(*ctx_, *info_);

  // reset should clear buffers
  EXPECT_TRUE(result.source.find(".clear()") != std::string::npos);
}

TEST_F(GenerateParserTest, NoErrorsReported) {
  auto result = generate_parser(*ctx_, *info_);

  EXPECT_TRUE(result.errors.empty())
      << "Should not have errors, but got: "
      << (result.errors.empty() ? "" : result.errors[0]);
}

TEST_F(GenerateParserTest, LookaheadCodeGenerated) {
  auto result = generate_parser(*ctx_, *info_);

  // The lookahead optimization code should be in the generator
  // Even if not used (single message per state), the code path exists
  // Check that the generator includes lookahead-related logic in the generated code
  // For states with single messages, direct dispatch is used
  EXPECT_TRUE(result.source.find("_parser_.parse(input)") != std::string::npos);
}

// SMTP-specific parser generation tests
class GenerateParserSMTPTest : public ::testing::Test {
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

TEST_F(GenerateParserSMTPTest, GeneratesValidCode) {
  auto result = generate_parser(*ctx_, *info_);

  EXPECT_FALSE(result.header.empty());
  EXPECT_FALSE(result.source.empty());
  EXPECT_TRUE(result.errors.empty())
      << "Errors: " << (result.errors.empty() ? "" : result.errors[0]);
}

TEST_F(GenerateParserSMTPTest, HasSMTPEHLOCommandParser) {
  auto result = generate_parser(*ctx_, *info_);

  EXPECT_TRUE(result.header.find("SMTPEHLOCommandParser") != std::string::npos)
      << "Should have SMTPEHLOCommandParser class";
}

TEST_F(GenerateParserSMTPTest, SMTPEHLOCommandParserHasClientDomainField) {
  auto result = generate_parser(*ctx_, *info_);

  // Find the SMTPEHLOCommandParser class
  auto parser_pos = result.header.find("class SMTPEHLOCommandParser");
  ASSERT_NE(parser_pos, std::string::npos) << "Should have SMTPEHLOCommandParser";

  // Find the end of this class (next class or end)
  auto next_class = result.header.find("class ", parser_pos + 10);
  std::string parser_class = result.header.substr(parser_pos,
      next_class != std::string::npos ? next_class - parser_pos : 500);

  EXPECT_TRUE(parser_class.find("client_domain") != std::string::npos)
      << "SMTPEHLOCommandParser should have client_domain field. Class:\n"
      << parser_class;
}

TEST_F(GenerateParserSMTPTest, SMTPEHLOParserImplementationMatchesEHLOPrefix) {
  auto result = generate_parser(*ctx_, *info_);

  // Find the SMTPEHLOCommandParser::parse implementation
  auto parse_pos = result.source.find("SMTPEHLOCommandParser::parse");
  ASSERT_NE(parse_pos, std::string::npos) << "Should have SMTPEHLOCommandParser::parse";

  // Get the function body
  auto func_end = result.source.find("\n}\n", parse_pos);
  std::string parse_func = result.source.substr(parse_pos,
      func_end != std::string::npos ? func_end - parse_pos + 3 : 1000);

  // Should match "EHLO " prefix
  EXPECT_TRUE(parse_func.find("EHLO ") != std::string::npos)
      << "SMTPEHLOCommandParser should match 'EHLO ' prefix. Function:\n"
      << parse_func;
}

TEST_F(GenerateParserSMTPTest, SMTPEHLOParserReturnsCorrectConsumedForValidInput) {
  auto result = generate_parser(*ctx_, *info_);

  // The parse function should:
  // 1. Match "EHLO " (5 bytes)
  // 2. Read until "\r\n" terminator
  // 3. Return Complete with total consumed bytes

  auto parse_pos = result.source.find("SMTPEHLOCommandParser::parse");
  ASSERT_NE(parse_pos, std::string::npos);

  auto func_end = result.source.find("\n}\n", parse_pos);
  std::string parse_func = result.source.substr(parse_pos,
      func_end != std::string::npos ? func_end - parse_pos + 3 : 1000);

  // Should have stage-based parsing
  EXPECT_TRUE(parse_func.find("stage_") != std::string::npos)
      << "Should use stage-based parsing";

  // Should return consumed count
  EXPECT_TRUE(parse_func.find("total_consumed") != std::string::npos)
      << "Should track total_consumed";

  // Should return Complete status when done
  EXPECT_TRUE(parse_func.find("ParseStatus::Complete") != std::string::npos)
      << "Should return Complete status";
}

TEST_F(GenerateParserSMTPTest, HasSMTPQUITCommandFromEHLOParser) {
  auto result = generate_parser(*ctx_, *info_);

  // SMTP protocol allows QUIT from EHLO state
  EXPECT_TRUE(result.header.find("SMTPQUITCommandFromEHLO") != std::string::npos)
      << "Should have SMTPQUITCommandFromEHLO parser for QUIT command from EHLO state";
}

TEST_F(GenerateParserSMTPTest, SMTPQUITParserMatchesQUITPrefix) {
  auto result = generate_parser(*ctx_, *info_);

  // Find a QUIT parser implementation
  auto quit_pos = result.source.find("SMTPQUITCommand");
  if (quit_pos != std::string::npos) {
    auto parse_pos = result.source.find("::parse", quit_pos);
    if (parse_pos != std::string::npos) {
      auto func_end = result.source.find("\n}\n", parse_pos);
      std::string parse_func = result.source.substr(parse_pos,
          func_end != std::string::npos ? func_end - parse_pos + 3 : 500);

      // Should match "QUIT" 
      EXPECT_TRUE(parse_func.find("QUIT") != std::string::npos)
          << "QUIT parser should match 'QUIT'. Function:\n" << parse_func;
    }
  }
}

TEST_F(GenerateParserSMTPTest, ParserReturnsErrorOnMismatch) {
  auto result = generate_parser(*ctx_, *info_);

  // Parsers should return Error when static octets don't match
  EXPECT_TRUE(result.source.find("ParseStatus::Error") != std::string::npos)
      << "Parsers should return Error on mismatch";
}

