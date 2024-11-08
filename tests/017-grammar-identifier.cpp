#include <networkprotocoldsl/lexer/token.hpp>
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