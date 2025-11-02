#ifndef INCLUDED_NETWORKPROTOCOLDSL_GRAMMAR_MESSAGE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_GRAMMAR_MESSAGE_HPP

#include <memory>
#include <optional>
#include <vector>

#include <networkprotocoldsl/lexer/token.hpp>
#include <networkprotocoldsl/parser/grammar/messagedata.hpp>
#include <networkprotocoldsl/parser/grammar/messagesequence.hpp>
#include <networkprotocoldsl/parser/grammar/traits.hpp>
#include <networkprotocoldsl/parser/support/recursiveparser.hpp>
#include <networkprotocoldsl/parser/tree/message.hpp>

namespace networkprotocoldsl::parser::grammar {

class Message
    : public support::RecursiveParser<Message, ParseTraits, Tracer<Message>> {
public:
  static constexpr const char *name = "Message";
  static void partial_match() {}
  static StringLiteral *recurse_one(lexer::token::keyword::Message) {
    return nullptr;
  }
  static void partial_match(lexer::token::keyword::Message,
                            std::shared_ptr<const tree::StringLiteral>) {}
  static MessageDataPair *
  recurse_many(lexer::token::keyword::Message,
               std::shared_ptr<const tree::StringLiteral>,
               lexer::token::punctuation::CurlyBraceOpen) {
    return nullptr;
  }
  static MessageData *
  recurse_maybe(lexer::token::keyword::Message,
                std::shared_ptr<const tree::StringLiteral>,
                lexer::token::punctuation::CurlyBraceOpen,
                std::vector<std::shared_ptr<const tree::MessageDataPair>>) {
    return nullptr;
  }
  static void
  partial_match(lexer::token::keyword::Message,
                std::shared_ptr<const tree::StringLiteral>,
                lexer::token::punctuation::CurlyBraceOpen,
                std::vector<std::shared_ptr<const tree::MessageDataPair>>,
                std::nullopt_t) {}

  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end, lexer::token::keyword::Message,
        std::shared_ptr<const tree::StringLiteral> messagename,
        lexer::token::punctuation::CurlyBraceOpen,
        std::vector<std::shared_ptr<const tree::MessageDataPair>> attributes,
        std::nullopt_t, lexer::token::punctuation::CurlyBraceClose) {
    auto message = std::make_shared<tree::Message>();
    message->name = messagename;
    message->data = std::make_shared<const tree::MessageData>();
    message->parts = std::make_shared<const tree::MessageSequence>();
    bool seen_when = false;
    bool seen_then = false;
    bool seen_agent = false;
    for (auto &attr : attributes) {
      if (attr->first == "when") {
        message->when = attr->second->name;
        seen_when = true;
      } else if (attr->first == "then") {
        message->then = attr->second->name;
        seen_then = true;
      } else if (attr->first == "agent") {
        message->agent = attr->second->name;
        seen_agent = true;
      }
    }
    if (!(seen_when && seen_then && seen_agent)) {
      return {std::nullopt, begin, end};
    } else {
      return {message, begin, end};
    }
  }

  static MessageParts *
  recurse_one(lexer::token::keyword::Message,
              std::shared_ptr<const tree::StringLiteral>,
              lexer::token::punctuation::CurlyBraceOpen,
              std::vector<std::shared_ptr<const tree::MessageDataPair>>,
              std::shared_ptr<const tree::MessageData>) {
    return nullptr;
  }
  static void
  partial_match(lexer::token::keyword::Message,
                std::shared_ptr<const tree::StringLiteral>,
                lexer::token::punctuation::CurlyBraceOpen,
                std::vector<std::shared_ptr<const tree::MessageDataPair>>,
                std::shared_ptr<const tree::MessageData>,
                std::shared_ptr<const tree::MessageSequence>) {}
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end, lexer::token::keyword::Message,
        std::shared_ptr<const tree::StringLiteral> messagename,
        lexer::token::punctuation::CurlyBraceOpen,
        std::vector<std::shared_ptr<const tree::MessageDataPair>> attributes,
        std::shared_ptr<const tree::MessageData> data,
        std::shared_ptr<const tree::MessageSequence> sequence,
        lexer::token::punctuation::CurlyBraceClose) {
    auto message = std::make_shared<tree::Message>();
    message->name = messagename;
    message->data = data;
    message->parts = sequence;
    bool seen_when = false;
    bool seen_then = false;
    bool seen_agent = false;
    for (auto &attr : attributes) {
      if (attr->first == "when") {
        message->when = attr->second->name;
        seen_when = true;
      } else if (attr->first == "then") {
        message->then = attr->second->name;
        seen_then = true;
      } else if (attr->first == "agent") {
        message->agent = attr->second->name;
        seen_agent = true;
      }
    }
    if (!(seen_when && seen_then && seen_agent)) {
      return {std::nullopt, begin, end};
    } else {
      return {message, begin, end};
    }
  };
};

} // namespace networkprotocoldsl::parser::grammar

#endif