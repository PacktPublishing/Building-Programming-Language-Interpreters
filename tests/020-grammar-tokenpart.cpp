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
