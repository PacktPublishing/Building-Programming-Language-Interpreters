#include <networkprotocoldsl/lexer/token.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>
#include <networkprotocoldsl/parser/grammar/messagedata.hpp>

#include <gtest/gtest.h>
#include <memory>
#include <vector>

using namespace networkprotocoldsl;

TEST(MessageDataTest, MessageDataFieldMatch) {
  auto maybe_tokens = lexer::tokenize("fieldName: FieldType;");
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result =
      parser::grammar::MessageDataPair::parse(tokens.cbegin(), tokens.cend());
  ASSERT_EQ(result.begin, tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  auto field = std::get<std::shared_ptr<const parser::tree::MessageDataPair>>(
      result.node.value());
  ASSERT_EQ(field->first, "fieldName");
  ASSERT_EQ(field->second->name->name, "FieldType");
}

TEST(MessageDataTest, MessageDataFieldWithParamsMatch) {
  auto maybe_tokens = lexer::tokenize("fieldName: FieldType<foo=42>;");
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result =
      parser::grammar::MessageDataPair::parse(tokens.cbegin(), tokens.cend());
  ASSERT_EQ(result.begin, tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  auto field = std::get<std::shared_ptr<const parser::tree::MessageDataPair>>(
      result.node.value());
  ASSERT_EQ(field->first, "fieldName");
  auto type = field->second;
  ASSERT_EQ(type->name->name, "FieldType");
  auto params = type->parameters;
  ASSERT_EQ(params->size(), 1);
  auto param = params->at("foo");
  auto value =
      std::get<std::shared_ptr<const parser::tree::IntegerLiteral>>(param);
  ASSERT_EQ(42, value->value);
}

TEST(MessageDataTest, MessageDataMatch) {
  auto maybe_tokens =
      lexer::tokenize("data: { param: int; other: foo<bar=42>; }");
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result =
      parser::grammar::MessageData::parse(tokens.cbegin(), tokens.cend());
  ASSERT_EQ(result.begin, tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  auto map = std::get<std::shared_ptr<const parser::tree::MessageData>>(
      result.node.value());
  auto p1 = map->at("param");
  ASSERT_EQ("int", p1->name->name);
  auto p2 = map->at("other");
  ASSERT_EQ("foo", p2->name->name);
  auto p2map = p2->parameters;
  auto p2v1 = p2map->at("bar");

  ASSERT_EQ(42,
            std::get<std::shared_ptr<const parser::tree::IntegerLiteral>>(p2v1)
                ->value);
}
