#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_LITERALS_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_LITERALS_HPP

#include <memory>

#include <networkprotocoldsl/lexer/token/literal.hpp>
#include <networkprotocoldsl/parser/grammar/traits.hpp>
#include <networkprotocoldsl/parser/support/recursiveparser.hpp>

namespace networkprotocoldsl::parser::grammar {

class BooleanLiteral
    : public support::RecursiveParser<BooleanLiteral, ParseTraits,
                                      Tracer<BooleanLiteral>> {
public:
  static constexpr const char *name = "BooleanLiteral";
  static void partial_match() {}
  static ParseStateReturn match(TokenIterator begin, TokenIterator end,
                                lexer::token::literal::Boolean b) {
    return {std::make_shared<tree::BooleanLiteral>(b.value), begin, end};
  }
};

class IntegerLiteral
    : public support::RecursiveParser<IntegerLiteral, ParseTraits,
                                      Tracer<IntegerLiteral>> {
public:
  static constexpr const char *name = "IntegerLiteral";
  static void partial_match() {}
  static ParseStateReturn match(TokenIterator begin, TokenIterator end,
                                lexer::token::literal::Integer i) {
    return {std::make_shared<tree::IntegerLiteral>(i.value), begin, end};
  }
};

class StringLiteral
    : public support::RecursiveParser<StringLiteral, ParseTraits,
                                      Tracer<StringLiteral>> {
public:
  static constexpr const char *name = "StringLiteral";
  static void partial_match() {}
  static ParseStateReturn match(TokenIterator begin, TokenIterator end,
                                lexer::token::literal::String s) {
    return {std::make_shared<tree::StringLiteral>(s.value), begin, end};
  }
};

} // namespace networkprotocoldsl::parser::grammar

#endif