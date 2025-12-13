#include <networkprotocoldsl/lexer/token.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>
#include <networkprotocoldsl/parser/grammar/tokenparts.hpp>

#include <gtest/gtest.h>
#include <memory>
#include <vector>

using namespace networkprotocoldsl;

TEST(TokenPartTest, SingleTokenPartMatchString) {
  auto maybe_tokens = lexer::tokenize("\"stringLiteral\"");
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result =
      parser::grammar::TokenPart::parse(tokens.cbegin(), tokens.cend());
  ASSERT_EQ(result.begin, tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  auto part = std::get<std::shared_ptr<const parser::tree::TokenPart>>(
      result.node.value());
  auto string_literal =
      std::get<std::shared_ptr<const parser::tree::StringLiteral>>(*part);
  ASSERT_EQ(string_literal->value, "stringLiteral");
}

TEST(TokenPartTest, SingleTokenPartMatchIdentifier) {
  auto maybe_tokens = lexer::tokenize("identifier");
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result =
      parser::grammar::TokenPart::parse(tokens.cbegin(), tokens.cend());
  ASSERT_EQ(result.begin, tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  auto part = std::get<std::shared_ptr<const parser::tree::TokenPart>>(
      result.node.value());
  auto id =
      std::get<std::shared_ptr<const parser::tree::IdentifierReference>>(*part);
  ASSERT_EQ(id->name, "identifier");
}

TEST(TokenPartTest, TokenSequence2) {
  auto maybe_tokens =
      lexer::tokenize("tokens { \"stringLiteral\" identifier }");
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result =
      parser::grammar::TokenSequence::parse(tokens.cbegin(), tokens.cend());
  ASSERT_EQ(result.begin, tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  auto seq = std::get<std::shared_ptr<const parser::tree::TokenSequence>>(
      result.node.value());
  ASSERT_EQ(seq->tokens.size(), 2);
  auto part1 = std::get<std::shared_ptr<const parser::tree::StringLiteral>>(
      *seq->tokens[0]);
  ASSERT_EQ("stringLiteral", part1->value);
  auto part2 =
      std::get<std::shared_ptr<const parser::tree::IdentifierReference>>(
          *seq->tokens[1]);
  ASSERT_EQ("identifier", part2->name);
}

TEST(TokenPartTest, Terminator) {
  auto maybe_tokens = lexer::tokenize("terminator { \";\" }");
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result =
      parser::grammar::Terminator::parse(tokens.cbegin(), tokens.cend());
  ASSERT_EQ(result.begin, tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  auto seq = std::get<std::shared_ptr<const parser::tree::Terminator>>(
      result.node.value());
  ASSERT_EQ(seq->value->value, ";");
}

TEST(TokenPartTest, EscapeReplacement) {
  // Test parsing escape=replace<"\n", "\r\n "> syntax
  auto maybe_tokens = lexer::tokenize(R"(replace<"\n", "\r\n ">)");
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result =
      parser::grammar::EscapeReplacement::parse(tokens.cbegin(), tokens.cend());
  ASSERT_EQ(result.begin, tokens.cend()) << "Should consume all tokens";
  ASSERT_TRUE(result.node.has_value()) << "Should parse successfully";
  auto escape = std::get<std::shared_ptr<parser::tree::EscapeReplacement>>(
      result.node.value());
  ASSERT_EQ("\n", escape->character) << "Escape character should be newline";
  ASSERT_EQ("\r\n ", escape->sequence) << "Escape sequence should be CRLF+space";
}

TEST(TokenPartTest, TokenSequenceOption) {
  // Test parsing terminator="\r\n" option
  auto maybe_tokens = lexer::tokenize(R"(terminator="\r\n")");
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result =
      parser::grammar::TokenSequenceOption::parse(tokens.cbegin(), tokens.cend());
  ASSERT_EQ(result.begin, tokens.cend()) << "Should consume all tokens";
  ASSERT_TRUE(result.node.has_value()) << "Should parse successfully";
  auto option = std::get<std::shared_ptr<const parser::tree::TokenSequenceOptionPair>>(
      result.node.value());
  ASSERT_EQ("terminator", option->first);
  auto terminator_value = std::get<std::string>(option->second);
  ASSERT_EQ("\r\n", terminator_value);
}

TEST(TokenPartTest, TokenSequenceOptionWithEscape) {
  // Test parsing escape=replace<"\n", "\r\n "> option
  auto maybe_tokens = lexer::tokenize(R"(escape=replace<"\n", "\r\n ">)");
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result =
      parser::grammar::TokenSequenceOption::parse(tokens.cbegin(), tokens.cend());
  ASSERT_EQ(result.begin, tokens.cend()) << "Should consume all tokens";
  ASSERT_TRUE(result.node.has_value()) << "Should parse successfully";
  auto option = std::get<std::shared_ptr<const parser::tree::TokenSequenceOptionPair>>(
      result.node.value());
  ASSERT_EQ("escape", option->first);
  auto escape_value = std::get<parser::tree::EscapeReplacement>(option->second);
  ASSERT_EQ("\n", escape_value.character);
  ASSERT_EQ("\r\n ", escape_value.sequence);
}

TEST(TokenPartTest, TokenSequenceWithOptions) {
  // Test parsing tokens<terminator="\r\n", escape=replace<"\n", "\r\n ">> { host }
  auto maybe_tokens = lexer::tokenize(
      R"(tokens<terminator="\r\n", escape=replace<"\n", "\r\n ">> { host })");
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result =
      parser::grammar::TokenSequence::parse(tokens.cbegin(), tokens.cend());
  ASSERT_EQ(result.begin, tokens.cend()) << "Should consume all tokens";
  ASSERT_TRUE(result.node.has_value()) << "Should parse successfully";
  auto seq = std::get<std::shared_ptr<const parser::tree::TokenSequence>>(
      result.node.value());
  
  // Check terminator option
  ASSERT_TRUE(seq->terminator.has_value()) << "Should have terminator option";
  ASSERT_EQ("\r\n", seq->terminator.value());
  
  // Check escape option
  ASSERT_TRUE(seq->escape.has_value()) << "Should have escape option";
  ASSERT_EQ("\n", seq->escape->character);
  ASSERT_EQ("\r\n ", seq->escape->sequence);
  
  // Should have one token part (the identifier "host")
  ASSERT_EQ(1, seq->tokens.size());
  auto part = std::get<std::shared_ptr<const parser::tree::IdentifierReference>>(
      *seq->tokens[0]);
  ASSERT_EQ("host", part->name);
}
