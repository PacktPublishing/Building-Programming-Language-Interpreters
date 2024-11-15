#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_IDENTIFIER_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_IDENTIFIER_HPP

#include <memory>

#include <networkprotocoldsl/lexer/token/identifier.hpp>
#include <networkprotocoldsl/parser/grammar/traits.hpp>
#include <networkprotocoldsl/parser/support/recursiveparser.hpp>
#include <networkprotocoldsl/parser/tree/identifierreference.hpp>

namespace networkprotocoldsl::parser::grammar {

class IdentifierReference; // Forward declaration

class IdentifierMemberAccess
    : public support::RecursiveParser<IdentifierMemberAccess, ParseTraits,
                                      Tracer<IdentifierMemberAccess>> {
public:
  static constexpr const char *name = "IdentiferMemberAccess";
  static void partial_match() {}
  static IdentifierReference *recurse_one(lexer::token::punctuation::Dot) {
    return nullptr;
  }
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end, lexer::token::punctuation::Dot,
        std::shared_ptr<const tree::IdentifierReference> member) {
    return {member, begin, end};
  }
};

class IdentifierReference
    : public support::RecursiveParser<IdentifierReference, ParseTraits,
                                      Tracer<IdentifierReference>> {
public:
  static constexpr const char *name = "IdentiferReference";
  static void partial_match() {}
  static IdentifierMemberAccess *recurse_maybe(lexer::token::Identifier) {
    return nullptr;
  }
  static ParseStateReturn match(TokenIterator begin, TokenIterator end,
                                lexer::token::Identifier i, std::nullopt_t) {
    return {std::make_shared<const tree::IdentifierReference>(i.name), begin,
            end};
  }
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end, lexer::token::Identifier i,
        std::shared_ptr<const tree::IdentifierReference> member) {
    return {std::make_shared<const tree::IdentifierReference>(i.name, member),
            begin, end};
  }
};

} // namespace networkprotocoldsl::parser::grammar

#endif