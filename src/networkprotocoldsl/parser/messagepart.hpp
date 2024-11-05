#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_MESSAGEPART_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_MESSAGEPART_HPP

#include <networkprotocoldsl/parser/terminator.hpp>
#include <networkprotocoldsl/parser/tokensequence.hpp>

#include <variant>

namespace networkprotocoldsl::parser {

class MessageForLoop;
using MessagePart = std::variant<TokenSequence, Terminator, MessageForLoop>;

} // namespace networkprotocoldsl::parser

#endif