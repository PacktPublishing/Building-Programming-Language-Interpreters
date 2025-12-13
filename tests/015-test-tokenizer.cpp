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
  std::string input("message \"HTTP \\\"Request\\\"\" { when: Open; }");
  auto output = lexer::tokenize(input);
  ASSERT_TRUE(output.has_value());
  ASSERT_EQ(8, output->size());

  ASSERT_TRUE(
      std::holds_alternative<lexer::token::keyword::Message>(output->at(0)));
  ASSERT_TRUE(
      std::holds_alternative<lexer::token::literal::String>(output->at(1)));
  ASSERT_EQ("HTTP \"Request\"",
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

TEST(tokenizer, escape_replace_syntax) {
  using namespace networkprotocoldsl;
  // Test tokenizing the escape=replace<"\n", "\r\n "> syntax
  std::string input(R"(tokens<terminator="\r\n", escape=replace<"\n", "\r\n ">> { host })");
  auto output = lexer::tokenize(input);
  ASSERT_TRUE(output.has_value()) << "Tokenization should succeed";
  
  // Expected tokens:
  // 0: tokens (keyword)
  // 1: <
  // 2: terminator (keyword - reused for options)
  // 3: =
  // 4: "\r\n"
  // 5: ,
  // 6: escape (identifier)
  // 7: =
  // 8: replace (identifier)
  // 9: <
  // 10: "\n"
  // 11: ,
  // 12: "\r\n "
  // 13: >
  // 14: >
  // 15: {
  // 16: host (identifier)
  // 17: }
  
  ASSERT_EQ(18, output->size()) << "Should have exactly 18 tokens";
  
  // tokens
  ASSERT_TRUE(std::holds_alternative<lexer::token::keyword::Tokens>(output->at(0)))
      << "Token 0 should be 'tokens' keyword";
  
  // <
  ASSERT_TRUE(std::holds_alternative<lexer::token::punctuation::AngleBracketOpen>(output->at(1)))
      << "Token 1 should be '<'";
  
  // terminator (the keyword is reused for options)
  ASSERT_TRUE(std::holds_alternative<lexer::token::keyword::Terminator>(output->at(2)))
      << "Token 2 should be 'terminator' keyword";
  
  // =
  ASSERT_TRUE(std::holds_alternative<lexer::token::punctuation::Equal>(output->at(3)))
      << "Token 3 should be '='";
  
  // "\r\n"
  ASSERT_TRUE(std::holds_alternative<lexer::token::literal::String>(output->at(4)))
      << "Token 4 should be string literal";
  ASSERT_EQ("\r\n", std::get<lexer::token::literal::String>(output->at(4)).value);
  
  // ,
  ASSERT_TRUE(std::holds_alternative<lexer::token::punctuation::Comma>(output->at(5)))
      << "Token 5 should be ','";
  
  // escape
  ASSERT_TRUE(std::holds_alternative<lexer::token::Identifier>(output->at(6)))
      << "Token 6 should be 'escape' identifier";
  ASSERT_EQ("escape", std::get<lexer::token::Identifier>(output->at(6)).name);
  
  // =
  ASSERT_TRUE(std::holds_alternative<lexer::token::punctuation::Equal>(output->at(7)))
      << "Token 7 should be '='";
  
  // replace
  ASSERT_TRUE(std::holds_alternative<lexer::token::Identifier>(output->at(8)))
      << "Token 8 should be 'replace' identifier";
  ASSERT_EQ("replace", std::get<lexer::token::Identifier>(output->at(8)).name);
  
  // <
  ASSERT_TRUE(std::holds_alternative<lexer::token::punctuation::AngleBracketOpen>(output->at(9)))
      << "Token 9 should be '<'";
  
  // "\n"
  ASSERT_TRUE(std::holds_alternative<lexer::token::literal::String>(output->at(10)))
      << "Token 10 should be string literal for escape char";
  ASSERT_EQ("\n", std::get<lexer::token::literal::String>(output->at(10)).value);
  
  // ,
  ASSERT_TRUE(std::holds_alternative<lexer::token::punctuation::Comma>(output->at(11)))
      << "Token 11 should be ','";
  
  // "\r\n "
  ASSERT_TRUE(std::holds_alternative<lexer::token::literal::String>(output->at(12)))
      << "Token 12 should be string literal for escape sequence";
  ASSERT_EQ("\r\n ", std::get<lexer::token::literal::String>(output->at(12)).value);
  
  // > (closing replace<...>)
  ASSERT_TRUE(std::holds_alternative<lexer::token::punctuation::AngleBracketClose>(output->at(13)))
      << "Token 13 should be '>'";
  
  // > (closing tokens<...>)
  ASSERT_TRUE(std::holds_alternative<lexer::token::punctuation::AngleBracketClose>(output->at(14)))
      << "Token 14 should be '>'";
  
  // {
  ASSERT_TRUE(std::holds_alternative<lexer::token::punctuation::CurlyBraceOpen>(output->at(15)))
      << "Token 15 should be '{'";
  
  // host
  ASSERT_TRUE(std::holds_alternative<lexer::token::Identifier>(output->at(16)))
      << "Token 16 should be 'host' identifier";
  ASSERT_EQ("host", std::get<lexer::token::Identifier>(output->at(16)).name);
  
  // }
  ASSERT_TRUE(std::holds_alternative<lexer::token::punctuation::CurlyBraceClose>(output->at(17)))
      << "Token 17 should be '}'";
}

