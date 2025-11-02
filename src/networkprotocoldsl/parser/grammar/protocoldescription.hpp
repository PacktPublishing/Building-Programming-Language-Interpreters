#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_PROTOCOLDESCRIPTION_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_PROTOCOLDESCRIPTION_HPP

#include <memory>
#include <optional>
#include <vector>

#include <networkprotocoldsl/parser/grammar/message.hpp>
#include <networkprotocoldsl/parser/grammar/traits.hpp>
#include <networkprotocoldsl/parser/support/recursiveparser.hpp>

#include <networkprotocoldsl/parser/tree/protocoldescription.hpp>

namespace networkprotocoldsl::parser::grammar {

class ProtocolDescription
    : public support::RecursiveParser<ProtocolDescription, ParseTraits,
                                      Tracer<ProtocolDescription>> {
public:
  static constexpr const char *name = "ProtocolDescription";
  static Message *recurse_many() { return nullptr; };
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end,
        std::vector<std::shared_ptr<const tree::Message>> messages) {
    auto protocol_description = std::make_shared<tree::ProtocolDescription>();
    for (auto &message : messages) {
      protocol_description->emplace(message->name->value, message);
    }
    return {protocol_description, begin, end};
  };
};

} // namespace networkprotocoldsl::parser::grammar

#endif