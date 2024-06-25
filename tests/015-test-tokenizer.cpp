#include <networkprotocoldsl/lexer/token.hpp>
#include <networkprotocoldsl/lexer/token/identifier.hpp>
#include <networkprotocoldsl/lexer/token/literal.hpp>
#include <networkprotocoldsl/lexer/token/punctuation.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>

#include <string>

#include <gtest/gtest.h>
#include <variant>

TEST(tokenizer, match) {
  using namespace networkprotocoldsl;
  std::string input("message \"HTTP Request\" { when: Open; }");
  auto output = lexer::tokenize(input);
  ASSERT_TRUE(output.has_value());
  ASSERT_EQ(8, output->size());

  ASSERT_TRUE(
      std::holds_alternative<lexer::token::keyword::Message>(output->at(0)));
  ASSERT_TRUE(
      std::holds_alternative<lexer::token::literal::String>(output->at(1)));
  ASSERT_EQ("HTTP Request",
            std::get<lexer::token::literal::String>(output->at(1)).value);
  ASSERT_TRUE(std::holds_alternative<lexer::token::punctuation::CurlyBraceOpen>(
      output->at(2)));
  ASSERT_TRUE(std::holds_alternative<lexer::token::Identifier>(output->at(3)));
  ASSERT_EQ("when", std::get<lexer::token::Identifier>(output->at(3)).name);
  ASSERT_TRUE(
      std::holds_alternative<lexer::token::punctuation::KeyValueSeparator>(
          output->at(4)));
  ASSERT_TRUE(std::holds_alternative<lexer::token::Identifier>(output->at(5)));
  ASSERT_EQ("Open", std::get<lexer::token::Identifier>(output->at(5)).name);
  ASSERT_TRUE(std::holds_alternative<lexer::token::punctuation::StatementEnd>(
      output->at(6)));
  ASSERT_TRUE(
      std::holds_alternative<lexer::token::punctuation::CurlyBraceClose>(
          output->at(7)));
}
