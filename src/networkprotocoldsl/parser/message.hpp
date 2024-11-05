#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_MESSAGE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_MESSAGE_HPP

#include <memory>
#include <networkprotocoldsl/parser/identifierreference.hpp>
#include <networkprotocoldsl/parser/messagedata.hpp>
#include <networkprotocoldsl/parser/messagesequence.hpp>

namespace networkprotocoldsl::parser {

struct Message {
  std::shared_ptr<const IdentifierReference> name;
  std::shared_ptr<const IdentifierReference> when;
  std::shared_ptr<const IdentifierReference> then;
  std::shared_ptr<const IdentifierReference> agent;
  std::shared_ptr<const MessageData> data;
  std::shared_ptr<const MessageSequence> parts;
};

} // namespace networkprotocoldsl::parser

#endif