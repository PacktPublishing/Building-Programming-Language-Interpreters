#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_MESSAGEFORLOOP_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_MESSAGEFORLOOP_HPP

#include <memory>
#include <networkprotocoldsl/parser/tree/identifierreference.hpp>
#include <networkprotocoldsl/parser/tree/messagesequence.hpp>

namespace networkprotocoldsl::parser::tree {

struct MessageSequence;
struct MessageForLoop {
  std::shared_ptr<const IdentifierReference> variable;
  std::shared_ptr<const IdentifierReference> collection;
  std::shared_ptr<const MessageSequence> block;
};

} // namespace networkprotocoldsl::parser::tree

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_MESSAGEFORLOOP_HPP