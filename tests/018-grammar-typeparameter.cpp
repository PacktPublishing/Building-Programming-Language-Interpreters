#include <networkprotocoldsl/lexer/token.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>
#include <networkprotocoldsl/parser/grammar/typeparameter.hpp>

#include <gtest/gtest.h>
#include <memory>
#include <vector>

using namespace networkprotocoldsl;

TEST(TypeParameterTest, TypeParameterValueMatchInteger) {
  auto maybe_tokens = lexer::tokenize("42");
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result = parser::grammar::TypeParameterValue::parse(tokens.cbegin(),
                                                           tokens.cend());
  ASSERT_EQ(result.begin, tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  ASSERT_EQ(std::get<std::shared_ptr<const parser::tree::IntegerLiteral>>(
                result.node.value())
                ->value,
            42);
}

TEST(TypeParameterTest, TypeParameterValueMatchBoolean) {
  auto maybe_tokens = lexer::tokenize("True");
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result = parser::grammar::TypeParameterValue::parse(tokens.cbegin(),
                                                           tokens.cend());
  ASSERT_EQ(result.begin, tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  ASSERT_EQ(std::get<std::shared_ptr<const parser::tree::BooleanLiteral>>(
                result.node.value())
                ->value,
            true);
}

TEST(TypeParameterTest, TypeParameterPairMatch) {
  auto maybe_tokens = lexer::tokenize("param=42");
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result =
      parser::grammar::TypeParameter::parse(tokens.cbegin(), tokens.cend());
  ASSERT_EQ(result.begin, tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  auto pair = std::get<std::shared_ptr<const parser::tree::TypeParameterPair>>(
      result.node.value());
  ASSERT_EQ(pair->first, "param");
  ASSERT_EQ(std::get<std::shared_ptr<const parser::tree::IntegerLiteral>>(
                pair->second)
                ->value,
            42);
}

TEST(TypeParameterTest, TypeParametersMatch) {
  auto maybe_tokens = lexer::tokenize("<param=42>");
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result =
      parser::grammar::TypeParameters::parse(tokens.cbegin(), tokens.cend());
  ASSERT_EQ(result.begin, tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  auto map = std::get<std::shared_ptr<const parser::tree::TypeParameterMap>>(
      result.node.value());
  auto param_v = map->at("param");
  ASSERT_EQ(
      std::get<std::shared_ptr<const parser::tree::IntegerLiteral>>(param_v)
          ->value,
      42);
}

TEST(TypeParameterTest, TypeParametersMatchMany) {
  auto maybe_tokens = lexer::tokenize("<param=42,other=True>");
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result =
      parser::grammar::TypeParameters::parse(tokens.cbegin(), tokens.cend());
  ASSERT_EQ(result.begin, tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  auto map = std::get<std::shared_ptr<const parser::tree::TypeParameterMap>>(
      result.node.value());
  auto param_v = map->at("param");
  ASSERT_EQ(
      std::get<std::shared_ptr<const parser::tree::IntegerLiteral>>(param_v)
          ->value,
      42);
  auto other_v = map->at("other");
  ASSERT_EQ(
      std::get<std::shared_ptr<const parser::tree::BooleanLiteral>>(other_v)
          ->value,
      true);
}

TEST(TypeParameterTest, TypeMatch) {
  auto maybe_tokens = lexer::tokenize("sometype");
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result = parser::grammar::Type::parse(tokens.cbegin(), tokens.cend());
  ASSERT_EQ(result.begin, tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  auto type =
      std::get<std::shared_ptr<const parser::tree::Type>>(result.node.value());
  ASSERT_EQ(type->name->name, "sometype");
}

TEST(TypeParameterTest, TypeMatchWithParameters) {
  auto maybe_tokens = lexer::tokenize("sometype<param=42>");
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result = parser::grammar::Type::parse(tokens.cbegin(), tokens.cend());
  ASSERT_EQ(result.begin, tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  auto type =
      std::get<std::shared_ptr<const parser::tree::Type>>(result.node.value());
  ASSERT_EQ(type->name->name, "sometype");
  auto param_v = type->parameters->at("param");
  ASSERT_EQ(
      std::get<std::shared_ptr<const parser::tree::IntegerLiteral>>(param_v)
          ->value,
      42);
}

TEST(TypeParameterTest, TypeMatchWithParametersRecurse) {
  auto maybe_tokens = lexer::tokenize("sometype<param=foo<bar=42>>");
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result = parser::grammar::Type::parse(tokens.cbegin(), tokens.cend());
  ASSERT_EQ(result.begin, tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  auto type =
      std::get<std::shared_ptr<const parser::tree::Type>>(result.node.value());
  ASSERT_EQ(type->name->name, "sometype");
  auto outer_map = type->parameters;
  auto param_v = outer_map->at("param");
  auto inner_t = std::get<std::shared_ptr<const parser::tree::Type>>(param_v);
  ASSERT_EQ(inner_t->name->name, "foo");
  auto inner_param_v = inner_t->parameters->at("bar");
  ASSERT_EQ(std::get<std::shared_ptr<const parser::tree::IntegerLiteral>>(
                inner_param_v)
                ->value,
            42);
}
