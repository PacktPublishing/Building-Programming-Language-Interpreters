#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_MESSAGEFORLOOP_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_MESSAGEFORLOOP_HPP

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <networkprotocoldsl/lexer/token.hpp>
#include <networkprotocoldsl/parser/grammar/identifier.hpp>
#include <networkprotocoldsl/parser/grammar/tokenparts.hpp>
#include <networkprotocoldsl/parser/grammar/traits.hpp>
#include <networkprotocoldsl/parser/support/recursiveparser.hpp>
#include <networkprotocoldsl/parser/tree/messageforloop.hpp>

namespace networkprotocoldsl::parser::grammar {

class MessageSequence; // Forward declaration

class MessageForLoop
    : public support::RecursiveParser<MessageForLoop, ParseTraits,
                                      Tracer<MessageForLoop>> {
public:
  static constexpr const char *name = "MessageForLoop";
  static void partial_match() {}
  // Try to parse optional options first
  static TokenSequenceOptions *recurse_maybe(lexer::token::keyword::For) {
    return nullptr;
  }
  // When no options provided (nullopt case) - go directly to recurse_one
  static IdentifierReference *recurse_one(lexer::token::keyword::For,
                                          std::nullopt_t) {
    return nullptr;
  }
  static void partial_match(lexer::token::keyword::For, std::nullopt_t,
                            std::shared_ptr<const tree::IdentifierReference>) {}
  static IdentifierReference *
  recurse_one(lexer::token::keyword::For, std::nullopt_t,
              std::shared_ptr<const tree::IdentifierReference>,
              lexer::token::keyword::In) {
    return nullptr;
  }
  static void partial_match(lexer::token::keyword::For, std::nullopt_t,
                            std::shared_ptr<const tree::IdentifierReference>,
                            lexer::token::keyword::In,
                            std::shared_ptr<const tree::IdentifierReference>) {}
  static MessageSequence *
  recurse_one(lexer::token::keyword::For, std::nullopt_t,
              std::shared_ptr<const tree::IdentifierReference>,
              lexer::token::keyword::In,
              std::shared_ptr<const tree::IdentifierReference>,
              lexer::token::punctuation::CurlyBraceOpen) {
    return nullptr;
  }
  static void partial_match(lexer::token::keyword::For, std::nullopt_t,
                            std::shared_ptr<const tree::IdentifierReference>,
                            lexer::token::keyword::In,
                            std::shared_ptr<const tree::IdentifierReference>,
                            lexer::token::punctuation::CurlyBraceOpen,
                            std::shared_ptr<const tree::MessageSequence>) {}
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end, lexer::token::keyword::For,
        std::nullopt_t,
        std::shared_ptr<const tree::IdentifierReference> identifier1,
        lexer::token::keyword::In,
        std::shared_ptr<const tree::IdentifierReference> identifier2,
        lexer::token::punctuation::CurlyBraceOpen,
        std::shared_ptr<const tree::MessageSequence> seq,
        lexer::token::punctuation::CurlyBraceClose) {
    return {std::make_shared<const tree::MessageForLoop>(tree::MessageForLoop{
                identifier1, identifier2, seq, std::nullopt}),
            begin, end};
  }
  // When options are provided - go directly to recurse_one
  static IdentifierReference *
  recurse_one(lexer::token::keyword::For,
              std::shared_ptr<tree::TokenSequenceOptionsMap>) {
    return nullptr;
  }
  static void partial_match(lexer::token::keyword::For,
                            std::shared_ptr<tree::TokenSequenceOptionsMap>,
                            std::shared_ptr<const tree::IdentifierReference>) {}
  static IdentifierReference *
  recurse_one(lexer::token::keyword::For,
              std::shared_ptr<tree::TokenSequenceOptionsMap>,
              std::shared_ptr<const tree::IdentifierReference>,
              lexer::token::keyword::In) {
    return nullptr;
  }
  static void partial_match(lexer::token::keyword::For,
                            std::shared_ptr<tree::TokenSequenceOptionsMap>,
                            std::shared_ptr<const tree::IdentifierReference>,
                            lexer::token::keyword::In,
                            std::shared_ptr<const tree::IdentifierReference>) {}
  static MessageSequence *
  recurse_one(lexer::token::keyword::For,
              std::shared_ptr<tree::TokenSequenceOptionsMap>,
              std::shared_ptr<const tree::IdentifierReference>,
              lexer::token::keyword::In,
              std::shared_ptr<const tree::IdentifierReference>,
              lexer::token::punctuation::CurlyBraceOpen) {
    return nullptr;
  }
  static void partial_match(lexer::token::keyword::For,
                            std::shared_ptr<tree::TokenSequenceOptionsMap>,
                            std::shared_ptr<const tree::IdentifierReference>,
                            lexer::token::keyword::In,
                            std::shared_ptr<const tree::IdentifierReference>,
                            lexer::token::punctuation::CurlyBraceOpen,
                            std::shared_ptr<const tree::MessageSequence>) {}
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end, lexer::token::keyword::For,
        std::shared_ptr<tree::TokenSequenceOptionsMap> options,
        std::shared_ptr<const tree::IdentifierReference> identifier1,
        lexer::token::keyword::In,
        std::shared_ptr<const tree::IdentifierReference> identifier2,
        lexer::token::punctuation::CurlyBraceOpen,
        std::shared_ptr<const tree::MessageSequence> seq,
        lexer::token::punctuation::CurlyBraceClose) {
    std::optional<std::string> terminator;
    if (options) {
      auto &opts = *options;
      if (opts.count("terminator")) {
        terminator = opts.at("terminator");
      }
    }
    return {std::make_shared<const tree::MessageForLoop>(
                tree::MessageForLoop{identifier1, identifier2, seq, terminator}),
            begin, end};
  }
};

class MessagePart : public support::RecursiveParser<MessagePart, ParseTraits,
                                                    Tracer<MessagePart>> {
public:
  static constexpr const char *name = "MessagePart";
  static std::tuple<TokenSequence, Terminator, MessageForLoop> *recurse_any() {
    return nullptr;
  }
  static ParseStateReturn match(TokenIterator begin, TokenIterator end,
                                tree::MessagePart part) {
    return {std::make_shared<const tree::MessagePart>(part), begin, end};
  }
};

class MessageSequence
    : public support::RecursiveParser<MessageSequence, ParseTraits,
                                      Tracer<MessageSequence>> {
public:
  static constexpr const char *name = "MessageSequence";
  static MessagePart *recurse_many() { return nullptr; }
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end,
        std::vector<std::shared_ptr<const tree::MessagePart>> parts) {
    return {std::make_shared<const tree::MessageSequence>(parts), begin, end};
  }
};

class MessageParts : public support::RecursiveParser<MessageParts, ParseTraits,
                                                     Tracer<MessageParts>> {
public:
  static constexpr const char *name = "MesageParts";
  static void partial_match() {}
  static void partial_match(lexer::token::keyword::Parts) {}
  static MessageSequence *
  recurse_one(lexer::token::keyword::Parts,
              lexer::token::punctuation::CurlyBraceOpen) {
    return nullptr;
  }
  static void partial_match(lexer::token::keyword::Parts,
                            lexer::token::punctuation::CurlyBraceOpen,
                            std::shared_ptr<const tree::MessageSequence>) {}
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end, lexer::token::keyword::Parts,
        lexer::token::punctuation::CurlyBraceOpen,
        std::shared_ptr<const tree::MessageSequence> parts,
        lexer::token::punctuation::CurlyBraceClose) {
    return {parts, begin, end};
  }
};

} // namespace networkprotocoldsl::parser::grammar

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_MESSAGEFORLOOP_HPP