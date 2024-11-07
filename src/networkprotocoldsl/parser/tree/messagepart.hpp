#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_MESSAGEPART_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_MESSAGEPART_HPP

#include <networkprotocoldsl/parser/tree/terminator.hpp>
#include <networkprotocoldsl/parser/tree/tokensequence.hpp>
#include <networkprotocoldsl/parser/tree/messageforloop.hpp>

#include <variant>

namespace networkprotocoldsl::parser::tree {

struct MessageForLoop;
using MessagePart = std::variant<TokenSequence, Terminator, MessageForLoop>;

}

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_MESSAGEPART_HPP