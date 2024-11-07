#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_MESSAGE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_MESSAGE_HPP

#include <memory>
#include <networkprotocoldsl/parser/tree/identifierreference.hpp>
#include <networkprotocoldsl/parser/tree/messagedata.hpp>
#include <networkprotocoldsl/parser/tree/messagesequence.hpp>

namespace networkprotocoldsl::parser::tree {

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