#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_MESSAGESEQUENCE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_MESSAGESEQUENCE_HPP

#include <memory>
#include <networkprotocoldsl/parser/messagepart.hpp>
#include <vector>

namespace networkprotocoldsl::parser {

using MessageSequence = std::vector<std::shared_ptr<const MessagePart>>;

}

#endif