#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_TOKENPARTS_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_TOKENPARTS_HPP

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <networkprotocoldsl/lexer/token.hpp>
#include <networkprotocoldsl/parser/grammar/identifier.hpp>
#include <networkprotocoldsl/parser/grammar/literals.hpp>
#include <networkprotocoldsl/parser/grammar/traits.hpp>
#include <networkprotocoldsl/parser/support/recursiveparser.hpp>
#include <networkprotocoldsl/parser/tree/tokenpart.hpp>
#include <networkprotocoldsl/parser/tree/tokensequenceoptions.hpp>

namespace networkprotocoldsl::parser::grammar {

class TokenPart : public support::RecursiveParser<TokenPart, ParseTraits,
                                                  Tracer<TokenPart>> {
public:
  static constexpr const char *name = "TokenPart";
  static std::tuple<StringLiteral, IdentifierReference> *recurse_any() {
    return nullptr;
  }
  static ParseStateReturn match(TokenIterator begin, TokenIterator end,
                                tree::TokenPart part) {
    return {std::make_shared<const tree::TokenPart>(part), begin, end};
  }
};

// Parses a single option like: terminator="\r\n" or escape="\r\n "
class TokenSequenceOption
    : public support::RecursiveParser<TokenSequenceOption, ParseTraits,
                                      Tracer<TokenSequenceOption>> {
public:
  static constexpr const char *name = "TokenSequenceOption";
  static void partial_match() {}
  static void partial_match(lexer::token::Identifier) {}
  static StringLiteral *recurse_one(lexer::token::Identifier,
                                    lexer::token::punctuation::Equal) {
    return nullptr;
  }
  static ParseStateReturn match(TokenIterator begin, TokenIterator end,
                                lexer::token::Identifier identifier,
                                lexer::token::punctuation::Equal,
                                std::shared_ptr<const tree::StringLiteral> value) {
    return {std::make_shared<const tree::TokenSequenceOptionPair>(identifier.name, value->value), begin, end};
  }
};

// Parses options block: < terminator="\r\n", escape="\r\n " >
class TokenSequenceOptions
    : public support::RecursiveParser<TokenSequenceOptions, ParseTraits,
                                      Tracer<TokenSequenceOptions>> {
public:
  static constexpr const char *name = "TokenSequenceOptions";
  static void partial_match() {}
  static TokenSequenceOption *
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
        std::vector<std::shared_ptr<const tree::TokenSequenceOptionPair>> m,
        lexer::token::punctuation::AngleBracketClose) {
    auto map = std::make_shared<tree::TokenSequenceOptionsMap>();
    for (auto &pair : m) {
      map->emplace(pair->first, pair->second);
    }
    return {map, begin, end};
  }
};

// TokenSequence with optional options: tokens < terminator="..." > { ... }
class TokenSequence
    : public support::RecursiveParser<TokenSequence, ParseTraits,
                                      Tracer<TokenSequence>> {
public:
  static constexpr const char *name = "TokenSequence";
  static void partial_match() {}
  static TokenSequenceOptions *recurse_maybe(lexer::token::keyword::Tokens) {
    return nullptr;
  }
  // When no options provided (nullopt case)
  static void partial_match(lexer::token::keyword::Tokens, std::nullopt_t) {}
  static TokenPart *recurse_many(lexer::token::keyword::Tokens,
                                 std::nullopt_t,
                                 lexer::token::punctuation::CurlyBraceOpen) {
    return nullptr;
  }
  static void
  partial_match(lexer::token::keyword::Tokens,
                std::nullopt_t,
                lexer::token::punctuation::CurlyBraceOpen,
                std::vector<std::shared_ptr<const tree::TokenPart>>) {}
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end, lexer::token::keyword::Tokens,
        std::nullopt_t,
        lexer::token::punctuation::CurlyBraceOpen,
        std::vector<std::shared_ptr<const tree::TokenPart>> parts,
        lexer::token::punctuation::CurlyBraceClose) {
    auto seq = std::make_shared<tree::TokenSequence>();
    seq->tokens = parts;
    return {seq, begin, end};
  }
  // When options are provided
  static void partial_match(lexer::token::keyword::Tokens,
                            std::shared_ptr<tree::TokenSequenceOptionsMap>) {}
  static TokenPart *recurse_many(lexer::token::keyword::Tokens,
                                 std::shared_ptr<tree::TokenSequenceOptionsMap>,
                                 lexer::token::punctuation::CurlyBraceOpen) {
    return nullptr;
  }
  static void
  partial_match(lexer::token::keyword::Tokens,
                std::shared_ptr<tree::TokenSequenceOptionsMap>,
                lexer::token::punctuation::CurlyBraceOpen,
                std::vector<std::shared_ptr<const tree::TokenPart>>) {}
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end, lexer::token::keyword::Tokens,
        std::shared_ptr<tree::TokenSequenceOptionsMap> options,
        lexer::token::punctuation::CurlyBraceOpen,
        std::vector<std::shared_ptr<const tree::TokenPart>> parts,
        lexer::token::punctuation::CurlyBraceClose) {
    auto seq = std::make_shared<tree::TokenSequence>();
    seq->tokens = parts;
    if (options) {
      auto &opts = *options;
      if (opts.count("terminator")) {
        seq->terminator = opts.at("terminator");
      }
      if (opts.count("escape")) {
        seq->escape = opts.at("escape");
      }
    }
    return {seq, begin, end};
  }
};

class Terminator : public support::RecursiveParser<Terminator, ParseTraits,
                                                   Tracer<Terminator>> {
public:
  static constexpr const char *name = "Terminator";
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