#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_MESSAGEFORLOOP_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_MESSAGEFORLOOP_HPP

#include <memory>
#include <networkprotocoldsl/parser/identifierreference.hpp>
#include <networkprotocoldsl/parser/messagesequence.hpp>

namespace networkprotocoldsl::parser {

struct MessageForLoop {
  std::shared_ptr<const IdentifierReference> variable;
  std::shared_ptr<const IdentifierReference> collection;
  std::shared_ptr<const MessageSequence> block;
};

} // namespace networkprotocoldsl::parser

#endif