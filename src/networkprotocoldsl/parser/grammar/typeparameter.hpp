#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_TYPEPARAMETER_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_TYPEPARAMETER_HPP

#include <memory>
#include <tuple>

#include <networkprotocoldsl/parser/grammar/traits.hpp>
#include <networkprotocoldsl/parser/support/recursiveparser.hpp>

#include <networkprotocoldsl/lexer/token.hpp>
#include <networkprotocoldsl/parser/grammar/identifier.hpp>
#include <networkprotocoldsl/parser/grammar/literals.hpp>

namespace networkprotocoldsl::parser::grammar {

class Type; // Forward declaration

class TypeParameterValue
    : public support::RecursiveParser<TypeParameterValue, ParseTraits> {
public:
  static std::tuple<Type, BooleanLiteral, StringLiteral, IntegerLiteral> *
  recurse_any() {
    return nullptr;
  }
  static ParseStateReturn match(TokenIterator begin, TokenIterator end,
                                tree::TypeParameterValue match) {
    return std::visit(
        [&](auto m) -> ParseStateReturn {
          return {m, begin, end};
        },
        match);
  }
};

class TypeParameter
    : public support::RecursiveParser<TypeParameter, ParseTraits> {
public:
  static void partial_match() {}
  static void partial_match(lexer::token::Identifier) {}
  static TypeParameterValue *recurse_one(lexer::token::Identifier,
                                         lexer::token::punctuation::Equal) {
    return nullptr;
  }
  static ParseStateReturn match(TokenIterator begin, TokenIterator end,
                                lexer::token::Identifier identifier,
                                lexer::token::punctuation::Equal,
                                tree::TypeParameterValue match) {
    return std::visit(
        [&](auto m) -> ParseStateReturn {
          return {std::make_shared<const tree::TypeParameterPair>(
                      identifier.name, m),
                  begin, end};
        },
        match);
  }
};

class TypeParameters
    : public support::RecursiveParser<TypeParameters, ParseTraits> {
public:
  static void partial_match() {}
  static TypeParameter *
  recurse_many(lexer::token::punctuation::AngleBracketOpen) {
    return nullptr;
  }
  static void
  recurse_many_has_separator(lexer::token::punctuation::AngleBracketOpen) {}
  static void
  recurse_many_separator(lexer::token::punctuation::AngleBracketOpen,
                         lexer::token::punctuation::Comma) {}
  static void partial_match(lexer::token::punctuation::AngleBracketOpen,
                            auto m) {}
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end,
        lexer::token::punctuation::AngleBracketOpen,
        std::vector<std::shared_ptr<const tree::TypeParameterPair>> m,
        lexer::token::punctuation::AngleBracketClose) {
    auto map = std::make_shared<tree::TypeParameterMap>();
    for (auto &pair : m) {
      map->emplace(pair->first, pair->second);
    }
    return {std::const_pointer_cast<const tree::TypeParameterMap>(map), begin,
            end};
  }
};

class Type : public support::RecursiveParser<Type, ParseTraits> {
public:
  static void partial_match() {}
  static TypeParameters *recurse_maybe(lexer::token::Identifier) {
    return nullptr;
  }
  static ParseStateReturn match(TokenIterator begin, TokenIterator end,
                                lexer::token::Identifier identifier,
                                std::nullopt_t) {
    auto id =
        std::make_shared<const tree::IdentifierReference>(identifier.name);
    return {std::make_shared<const tree::Type>(
                id, std::make_shared<const tree::TypeParameterMap>()),
            begin, end};
  }
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end,
        lexer::token::Identifier identifier,
        std::shared_ptr<const tree::TypeParameterMap> parameters) {
    auto id =
        std::make_shared<const tree::IdentifierReference>(identifier.name);
    return {std::make_shared<const tree::Type>(id, parameters), begin, end};
  }
};

} // namespace networkprotocoldsl::parser::grammar

#endif