#include <networkprotocoldsl/lexer/token.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>
#include <networkprotocoldsl/parser/grammar/identifier.hpp>

#include <gtest/gtest.h>
#include <memory>
#include <vector>

using namespace networkprotocoldsl;

TEST(IdentifierReferenceTest, IdentifierReferenceMatch) {
  std::vector<lexer::Token> tokens = {lexer::token::Identifier("myIdentifier")};
  auto result = parser::grammar::IdentifierReference::parse(tokens.cbegin(),
                                                            tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  ASSERT_EQ(std::get<std::shared_ptr<const parser::tree::IdentifierReference>>(
                result.node.value())
                ->name,
            "myIdentifier");
}

TEST(IdentifierReferenceTest, IdentifierReferenceWithMemberMatch) {
  auto maybe_tokens = lexer::tokenize("myIdentifier.member");
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> tokens = maybe_tokens.value();
  auto result = parser::grammar::IdentifierReference::parse(tokens.cbegin(),
                                                            tokens.cend());
  ASSERT_TRUE(result.node.has_value());
  auto id = std::get<std::shared_ptr<const parser::tree::IdentifierReference>>(
      result.node.value());
  ASSERT_EQ(id->name, "myIdentifier");
  ASSERT_TRUE(id->member.has_value());
  ASSERT_EQ(id->member.value()->name, "member");
}
