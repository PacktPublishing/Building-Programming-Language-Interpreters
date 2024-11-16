#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_MESSAGEPART_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_MESSAGEPART_HPP

#include <networkprotocoldsl/parser/tree/messageforloop.hpp>
#include <networkprotocoldsl/parser/tree/terminator.hpp>
#include <networkprotocoldsl/parser/tree/tokensequence.hpp>

#include <variant>

namespace networkprotocoldsl::parser::tree {

struct MessageForLoop;
using MessagePart = std::variant<std::shared_ptr<const TokenSequence>,
                                 std::shared_ptr<const Terminator>,
                                 std::shared_ptr<const MessageForLoop>>;




} // namespace networkprotocoldsl::parser::tree

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_MESSAGEPART_HPP