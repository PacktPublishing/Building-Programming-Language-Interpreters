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

// Parses escape replacement: replace<"\n", "\r\n ">
class EscapeReplacement
    : public support::RecursiveParser<EscapeReplacement, ParseTraits,
                                      Tracer<EscapeReplacement>> {
public:
  static constexpr const char *name = "EscapeReplacement";
  static void partial_match() {}
  // Must start with identifier "replace"
  static bool conditional_partial_match(lexer::token::Identifier id) {
    return id.name == "replace";
  }
  static void partial_match(lexer::token::Identifier,
                            lexer::token::punctuation::AngleBracketOpen) {}
  static void partial_match(lexer::token::Identifier,
                            lexer::token::punctuation::AngleBracketOpen,
                            lexer::token::literal::String) {}
  static void partial_match(lexer::token::Identifier,
                            lexer::token::punctuation::AngleBracketOpen,
                            lexer::token::literal::String,
                            lexer::token::punctuation::Comma) {}
  static void partial_match(lexer::token::Identifier,
                            lexer::token::punctuation::AngleBracketOpen,
                            lexer::token::literal::String,
                            lexer::token::punctuation::Comma,
                            lexer::token::literal::String) {}
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end,
        lexer::token::Identifier,
        lexer::token::punctuation::AngleBracketOpen,
        lexer::token::literal::String character,
        lexer::token::punctuation::Comma,
        lexer::token::literal::String sequence,
        lexer::token::punctuation::AngleBracketClose) {
    auto result = std::make_shared<tree::EscapeReplacement>();
    result->character = character.value;
    result->sequence = sequence.value;
    return {result, begin, end};
  }
};

// Parses option value: either a string literal or an escape replacement
class TokenSequenceOptionValue
    : public support::RecursiveParser<TokenSequenceOptionValue, ParseTraits,
                                      Tracer<TokenSequenceOptionValue>> {
public:
  static constexpr const char *name = "TokenSequenceOptionValue";
  static std::tuple<StringLiteral, EscapeReplacement> *recurse_any() {
    return nullptr;
  }
  static ParseStateReturn match(TokenIterator begin, TokenIterator end,
                                std::shared_ptr<const tree::StringLiteral> str) {
    auto result = std::make_shared<tree::TokenSequenceOptionValue>(str->value);
    return {result, begin, end};
  }
  static ParseStateReturn match(TokenIterator begin, TokenIterator end,
                                std::shared_ptr<tree::EscapeReplacement> esc) {
    auto result = std::make_shared<tree::TokenSequenceOptionValue>(*esc);
    return {result, begin, end};
  }
};

// Parses a single option like: terminator="\r\n" or escape=replace<"\n", "\r\n ">
// Note: "terminator" is tokenized as a keyword, so we need to handle both
// Identifier and Terminator keyword as the option name
class TokenSequenceOption
    : public support::RecursiveParser<TokenSequenceOption, ParseTraits,
                                      Tracer<TokenSequenceOption>> {
public:
  static constexpr const char *name = "TokenSequenceOption";
  static void partial_match() {}
  // Handle identifier as option name (e.g., "escape")
  static void partial_match(lexer::token::Identifier) {}
  static TokenSequenceOptionValue *recurse_one(lexer::token::Identifier,
                                               lexer::token::punctuation::Equal) {
    return nullptr;
  }
  static ParseStateReturn match(TokenIterator begin, TokenIterator end,
                                lexer::token::Identifier identifier,
                                lexer::token::punctuation::Equal,
                                std::shared_ptr<tree::TokenSequenceOptionValue> value) {
    return {std::make_shared<const tree::TokenSequenceOptionPair>(identifier.name, *value), begin, end};
  }
  // Handle "terminator" keyword as option name
  static void partial_match(lexer::token::keyword::Terminator) {}
  static TokenSequenceOptionValue *recurse_one(lexer::token::keyword::Terminator,
                                               lexer::token::punctuation::Equal) {
    return nullptr;
  }
  static ParseStateReturn match(TokenIterator begin, TokenIterator end,
                                lexer::token::keyword::Terminator,
                                lexer::token::punctuation::Equal,
                                std::shared_ptr<tree::TokenSequenceOptionValue> value) {
    return {std::make_shared<const tree::TokenSequenceOptionPair>("terminator", *value), begin, end};
  }
};

// Parses options block: < terminator="\r\n", escape=replace<"\n", "\r\n "> >
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
                            std::shared_ptr<const tree::TokenSequenceOptionsMap>) {}
  static TokenPart *recurse_many(lexer::token::keyword::Tokens,
                                 std::shared_ptr<const tree::TokenSequenceOptionsMap>,
                                 lexer::token::punctuation::CurlyBraceOpen) {
    return nullptr;
  }
  static void
  partial_match(lexer::token::keyword::Tokens,
                std::shared_ptr<const tree::TokenSequenceOptionsMap>,
                lexer::token::punctuation::CurlyBraceOpen,
                std::vector<std::shared_ptr<const tree::TokenPart>>) {}
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end, lexer::token::keyword::Tokens,
        std::shared_ptr<const tree::TokenSequenceOptionsMap> options,
        lexer::token::punctuation::CurlyBraceOpen,
        std::vector<std::shared_ptr<const tree::TokenPart>> parts,
        lexer::token::punctuation::CurlyBraceClose) {
    auto seq = std::make_shared<tree::TokenSequence>();
    seq->tokens = parts;
    if (options) {
      auto &opts = *options;
      if (opts.count("terminator")) {
        auto &val = opts.at("terminator");
        if (std::holds_alternative<std::string>(val)) {
          seq->terminator = std::get<std::string>(val);
        }
      }
      if (opts.count("escape")) {
        auto &val = opts.at("escape");
        if (std::holds_alternative<tree::EscapeReplacement>(val)) {
          seq->escape = std::get<tree::EscapeReplacement>(val);
        }
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