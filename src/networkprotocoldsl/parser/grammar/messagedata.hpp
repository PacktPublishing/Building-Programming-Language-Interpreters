#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_MESSAGEDATA_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_MESSAGEDATA_HPP

#include <memory>

#include <networkprotocoldsl/lexer/token.hpp>
#include <networkprotocoldsl/parser/grammar/identifier.hpp>
#include <networkprotocoldsl/parser/grammar/traits.hpp>
#include <networkprotocoldsl/parser/grammar/typeparameter.hpp>
#include <networkprotocoldsl/parser/support/recursiveparser.hpp>
#include <networkprotocoldsl/parser/tree/messagedata.hpp>

namespace networkprotocoldsl::parser::grammar {

class MessageDataPair
    : public support::RecursiveParser<MessageDataPair, ParseTraits,
                                      Tracer<MessageDataPair>> {
public:
  static constexpr const char *name = "MessageDataPair";
  static void partial_match() {}
  static void partial_match(lexer::token::Identifier) {}
  static Type *recurse_one(lexer::token::Identifier,
                           lexer::token::punctuation::KeyValueSeparator) {
    return nullptr;
  }
  static void partial_match(lexer::token::Identifier,
                            lexer::token::punctuation::KeyValueSeparator,
                            std::shared_ptr<const tree::Type>) {}
  static ParseStateReturn match(TokenIterator begin, TokenIterator end,
                                lexer::token::Identifier identifier,
                                lexer::token::punctuation::KeyValueSeparator,
                                std::shared_ptr<const tree::Type> type,
                                lexer::token::punctuation::StatementEnd) {
    auto pair =
        std::make_shared<const tree::MessageDataPair>(identifier.name, type);
    return {pair, begin, end};
  }
};

class MessageData : public support::RecursiveParser<MessageData, ParseTraits,
                                                    Tracer<MessageData>> {
public:
  static constexpr const char *name = "MessageData";
  static void partial_match() {}
  static bool conditional_partial_match(lexer::token::Identifier id) {
    if (id.name == "data") {
      return true;
    }
    return false;
  }
  static void partial_match(lexer::token::Identifier,
                            lexer::token::punctuation::KeyValueSeparator) {}
  static MessageDataPair *
  recurse_many(lexer::token::Identifier,
               lexer::token::punctuation::KeyValueSeparator,
               lexer::token::punctuation::CurlyBraceOpen) {
    return nullptr;
  }
  static void
  partial_match(lexer::token::Identifier,
                lexer::token::punctuation::KeyValueSeparator,
                lexer::token::punctuation::CurlyBraceOpen,
                std::vector<std::shared_ptr<const tree::MessageDataPair>>) {}
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end,
        lexer::token::Identifier identifier,
        lexer::token::punctuation::KeyValueSeparator,
        lexer::token::punctuation::CurlyBraceOpen,
        std::vector<std::shared_ptr<const tree::MessageDataPair>> fields,
        lexer::token::punctuation::CurlyBraceClose) {
    auto map = std::make_shared<tree::MessageData>();
    for (auto &pair : fields) {
      map->emplace(pair->first, pair->second);
    }
    return {map, begin, end};
  }
};

} // namespace networkprotocoldsl::parser::grammar

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_MESSAGEDATA_HPP