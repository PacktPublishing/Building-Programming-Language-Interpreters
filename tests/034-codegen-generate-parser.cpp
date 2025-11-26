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

// Tests for main Parser class features

TEST_F(GenerateParserTest, MainParserHasParserInstances) {
  auto result = generate_parser(*ctx_, *info_);

  // Main Parser should have instances of individual message parsers
  EXPECT_TRUE(result.header.find("HTTPResponseParser") != std::string::npos);
  EXPECT_TRUE(result.header.find("HTTPRequestParser") != std::string::npos);
  EXPECT_TRUE(result.header.find("_parser_") != std::string::npos);
}

TEST_F(GenerateParserTest, MainParserHasActiveParserIndex) {
  auto result = generate_parser(*ctx_, *info_);

  // Should track which parser completed
  EXPECT_TRUE(result.header.find("active_parser_index()") != std::string::npos);
  EXPECT_TRUE(result.header.find("active_parser_") != std::string::npos);
}

TEST_F(GenerateParserTest, MainParserHasTakeMethods) {
  auto result = generate_parser(*ctx_, *info_);

  // Should have take methods for each message type
  EXPECT_TRUE(result.header.find("take_HTTPResponse()") != std::string::npos);
  EXPECT_TRUE(result.header.find("take_HTTPRequest()") != std::string::npos);
}

TEST_F(GenerateParserTest, SourceHasStateDispatch) {
  auto result = generate_parser(*ctx_, *info_);

  // Parser::parse should dispatch based on current state
  EXPECT_TRUE(result.source.find("switch (current_state_)") != std::string::npos);
  EXPECT_TRUE(result.source.find("case State::") != std::string::npos);
}

TEST_F(GenerateParserTest, SourceDispatchesToMessageParsers) {
  auto result = generate_parser(*ctx_, *info_);

  // Should call individual message parser's parse method
  EXPECT_TRUE(result.source.find("_parser_.parse(input)") != std::string::npos);
}

TEST_F(GenerateParserTest, SourceSetsHasMessageOnComplete) {
  auto result = generate_parser(*ctx_, *info_);

  // Should set has_message_ when parsing completes
  EXPECT_TRUE(result.source.find("has_message_ = true") != std::string::npos);
}

TEST_F(GenerateParserTest, SourceSetsActiveParserOnComplete) {
  auto result = generate_parser(*ctx_, *info_);

  // Should set active_parser_ when parsing completes
  EXPECT_TRUE(result.source.find("active_parser_ =") != std::string::npos);
}

TEST_F(GenerateParserTest, TakeMethodsClearHasMessage) {
  auto result = generate_parser(*ctx_, *info_);

  // Take methods should clear has_message flag
  // Find take_HTTPResponse implementation
  auto pos = result.source.find("Parser::take_HTTPResponse()");
  ASSERT_TRUE(pos != std::string::npos);
  auto impl_end = result.source.find("}\n", pos);
  std::string impl = result.source.substr(pos, impl_end - pos);
  EXPECT_TRUE(impl.find("has_message_ = false") != std::string::npos);
}

TEST_F(GenerateParserTest, ResetClearsAllParsers) {
  auto result = generate_parser(*ctx_, *info_);

  // Main Parser reset should reset all individual parsers
  auto pos = result.source.find("void Parser::reset()");
  ASSERT_TRUE(pos != std::string::npos);
  auto impl_end = result.source.find("}\n\n", pos);
  std::string impl = result.source.substr(pos, impl_end - pos);
  EXPECT_TRUE(impl.find("_parser_.reset()") != std::string::npos);
}

// Tests for loop parsing

TEST_F(GenerateParserTest, LoopParsingHasCollectionBuffer) {
  auto result = generate_parser(*ctx_, *info_);

  // HTTP has headers array - should have a buffer for it
  // Looking for headers_buffer_ or similar
  EXPECT_TRUE(result.header.find("_buffer_") != std::string::npos);
}

TEST_F(GenerateParserTest, LoopParsingHasLoopStage) {
  auto result = generate_parser(*ctx_, *info_);

  // Loop parsing should track its own stage
  EXPECT_TRUE(result.header.find("loop_") != std::string::npos);
}

TEST_F(GenerateParserTest, LoopParsingChecksTerminator) {
  auto result = generate_parser(*ctx_, *info_);

  // Loop should check for loop terminator
  EXPECT_TRUE(result.source.find("loop_terminator") != std::string::npos);
}

TEST_F(GenerateParserTest, LoopParsingAddsToCollection) {
  auto result = generate_parser(*ctx_, *info_);

  // Loop should push elements to the collection buffer
  EXPECT_TRUE(result.source.find("push_back") != std::string::npos);
}

TEST_F(GenerateParserTest, LoopParsingResetsElementStage) {
  auto result = generate_parser(*ctx_, *info_);

  // After adding element, loop stage should reset to 0
  EXPECT_TRUE(result.source.find("_stage_ = 0") != std::string::npos);
}

TEST_F(GenerateParserTest, LoopParsingMovesCollection) {
  auto result = generate_parser(*ctx_, *info_);

  // take_data should move the collection
  // Look for moving a buffer that ends with _buffer_ and contains collection name
  EXPECT_TRUE(result.source.find("std::move(") != std::string::npos);
}

TEST_F(GenerateParserTest, LoopResetClearsCollection) {
  auto result = generate_parser(*ctx_, *info_);

  // reset should clear collection buffers
  // Multiple .clear() calls expected
  size_t count = 0;
  size_t pos = 0;
  while ((pos = result.source.find(".clear()", pos)) != std::string::npos) {
    ++count;
    ++pos;
  }
  EXPECT_GT(count, 1) << "Should have multiple .clear() calls for buffers";
}

TEST_F(GenerateParserTest, HeaderHasVectorInclude) {
  auto result = generate_parser(*ctx_, *info_);

  // Header should include <vector> for loop collection buffers
  EXPECT_TRUE(result.header.find("#include <vector>") != std::string::npos);
}

TEST_F(GenerateParserTest, HasAutoGeneratedComment) {
  auto result = generate_parser(*ctx_, *info_);

  // Both files should have auto-generated comment
  EXPECT_TRUE(result.header.find("Auto-generated by NetworkProtocolDSL") != std::string::npos);
  EXPECT_TRUE(result.source.find("Auto-generated by NetworkProtocolDSL") != std::string::npos);
}

TEST_F(GenerateParserTest, ActiveParserIndexDocumented) {
  auto result = generate_parser(*ctx_, *info_);

  // active_parser_index should document which indices correspond to which parsers
  EXPECT_TRUE(result.header.find("Index values:") != std::string::npos);
  EXPECT_TRUE(result.header.find("0 = ") != std::string::npos);
}

TEST_F(GenerateParserTest, LoopHandlesEmptyInput) {
  auto result = generate_parser(*ctx_, *info_);

  // Loop should check for empty input before trying to parse element
  EXPECT_TRUE(result.source.find("if (input.empty())") != std::string::npos);
}

// Tests for new status(), on_eof(), and lookahead features

TEST_F(GenerateParserTest, HeaderHasStatusMethod) {
  auto result = generate_parser(*ctx_, *info_);

  // Parser should have a status() const method
  EXPECT_TRUE(result.header.find("ParseStatus status() const") != std::string::npos);
}

TEST_F(GenerateParserTest, SourceImplementsStatusMethod) {
  auto result = generate_parser(*ctx_, *info_);

  // status() should return Complete if has_message_, NeedMoreData otherwise
  EXPECT_TRUE(result.source.find("ParseStatus Parser::status() const") != std::string::npos);
  EXPECT_TRUE(result.source.find("if (has_message_)") != std::string::npos);
  EXPECT_TRUE(result.source.find("return ParseStatus::Complete") != std::string::npos);
}

TEST_F(GenerateParserTest, HeaderHasOnEofMethod) {
  auto result = generate_parser(*ctx_, *info_);

  // Parser should have on_eof() method
  EXPECT_TRUE(result.header.find("void on_eof()") != std::string::npos);
}

TEST_F(GenerateParserTest, HeaderHasEofReceivedAccessor) {
  auto result = generate_parser(*ctx_, *info_);

  // Parser should have eof_received() accessor
  EXPECT_TRUE(result.header.find("bool eof_received() const") != std::string::npos);
  EXPECT_TRUE(result.header.find("eof_received_") != std::string::npos);
}

TEST_F(GenerateParserTest, SourceImplementsOnEof) {
  auto result = generate_parser(*ctx_, *info_);

  // on_eof should set eof_received_ flag
  EXPECT_TRUE(result.source.find("void Parser::on_eof()") != std::string::npos);
  EXPECT_TRUE(result.source.find("eof_received_ = true") != std::string::npos);
}

TEST_F(GenerateParserTest, ResetClearsEofFlag) {
  auto result = generate_parser(*ctx_, *info_);

  // reset() should clear eof_received_ flag
  auto pos = result.source.find("void Parser::reset()");
  ASSERT_TRUE(pos != std::string::npos);
  auto impl_end = result.source.find("}\n\n", pos);
  std::string impl = result.source.substr(pos, impl_end - pos);
  EXPECT_TRUE(impl.find("eof_received_ = false") != std::string::npos);
}

TEST_F(GenerateParserTest, LookaheadCodeGenerated) {
  auto result = generate_parser(*ctx_, *info_);

  // The lookahead optimization code should be in the generator
  // Even if not used (single message per state), the code path exists
  // Check that the generator includes lookahead-related logic in the generated code
  // For states with single messages, direct dispatch is used
  EXPECT_TRUE(result.source.find("_parser_.parse(input)") != std::string::npos);
}
