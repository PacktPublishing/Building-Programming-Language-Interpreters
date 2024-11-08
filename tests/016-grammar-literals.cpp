#include <networkprotocoldsl/lexer/token.hpp>
#include <networkprotocoldsl/parser/grammar/literals.hpp>

#include <gtest/gtest.h>
#include <memory>
#include <vector>

using namespace networkprotocoldsl;

TEST(LiteralsTest, BooleanLiteralMatchTrue) {
  std::vector<lexer::Token> tokens = {lexer::token::literal::Boolean(true)};
  auto result =
      parser::grammar::BooleanLiteral::parse(tokens.cbegin(), tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  ASSERT_EQ(std::get<std::shared_ptr<const parser::tree::BooleanLiteral>>(
                result.node.value())
                ->value,
            true);
}

TEST(LiteralsTest, BooleanLiteralMatchFalse) {
  std::vector<lexer::Token> tokens = {lexer::token::literal::Boolean(false)};
  auto result =
      parser::grammar::BooleanLiteral::parse(tokens.cbegin(), tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  ASSERT_EQ(std::get<std::shared_ptr<const parser::tree::BooleanLiteral>>(
                result.node.value())
                ->value,
            false);
}

TEST(LiteralsTest, IntegerLiteralMatch) {
  std::vector<lexer::Token> tokens = {lexer::token::literal::Integer(42)};
  auto result =
      parser::grammar::IntegerLiteral::parse(tokens.cbegin(), tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  ASSERT_EQ(std::get<std::shared_ptr<const parser::tree::IntegerLiteral>>(
                result.node.value())
                ->value,
            42);
}

TEST(LiteralsTest, StringLiteralMatch) {
  std::vector<lexer::Token> tokens = {lexer::token::literal::String("hello")};
  auto result =
      parser::grammar::StringLiteral::parse(tokens.cbegin(), tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  ASSERT_EQ(std::get<std::shared_ptr<const parser::tree::StringLiteral>>(
                result.node.value())
                ->value,
            "hello");
}