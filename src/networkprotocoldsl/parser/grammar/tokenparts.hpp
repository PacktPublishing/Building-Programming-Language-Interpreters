#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_TOKENPARTS_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_TOKENPARTS_HPP

#include <memory>
#include <vector>

#include <networkprotocoldsl/lexer/token.hpp>
#include <networkprotocoldsl/parser/grammar/identifier.hpp>
#include <networkprotocoldsl/parser/grammar/literals.hpp>
#include <networkprotocoldsl/parser/grammar/traits.hpp>
#include <networkprotocoldsl/parser/support/recursiveparser.hpp>
#include <networkprotocoldsl/parser/tree/tokenpart.hpp>

namespace networkprotocoldsl::parser::grammar {

class TokenPart : public support::RecursiveParser<TokenPart, ParseTraits> {
public:
  static std::tuple<StringLiteral, IdentifierReference> *recurse_any() {
    return nullptr;
  }
  static ParseStateReturn match(TokenIterator begin, TokenIterator end,
                                tree::TokenPart part) {
    return {std::make_shared<const tree::TokenPart>(part), begin, end};
  }
};

class TokenSequence
    : public support::RecursiveParser<TokenSequence, ParseTraits> {
public:
  static void partial_match() {}
  static void partial_match(lexer::token::keyword::Tokens) {}
  static TokenPart *recurse_many(lexer::token::keyword::Tokens,
                                 lexer::token::punctuation::CurlyBraceOpen) {
    return nullptr;
  }
  static void
  partial_match(lexer::token::keyword::Tokens,
                lexer::token::punctuation::CurlyBraceOpen,
                std::vector<std::shared_ptr<const tree::TokenPart>>) {}
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end, lexer::token::keyword::Tokens,
        lexer::token::punctuation::CurlyBraceOpen,
        std::vector<std::shared_ptr<const tree::TokenPart>> parts,
        lexer::token::punctuation::CurlyBraceClose) {
    return {std::make_shared<const tree::TokenSequence>(parts), begin, end};
  }
};

class Terminator : public support::RecursiveParser<Terminator, ParseTraits> {
public:
  static void partial_match() {}
  static void partial_match(lexer::token::keyword::Terminator) {}
  static StringLiteral *recurse_one(lexer::token::keyword::Terminator,
                                    lexer::token::punctuation::CurlyBraceOpen) {
    return nullptr;
  }
  static void partial_match(lexer::token::keyword::Terminator,
                            lexer::token::punctuation::CurlyBraceOpen,
                            std::shared_ptr<const tree::StringLiteral>) {}
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end,
        lexer::token::keyword::Terminator,
        lexer::token::punctuation::CurlyBraceOpen,
        std::shared_ptr<const tree::StringLiteral> literal,
        lexer::token::punctuation::CurlyBraceClose) {
    return {std::make_shared<const tree::Terminator>(literal), begin, end};
  }
};

} // namespace networkprotocoldsl::parser::grammar

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_TOKENPARTS_HPP