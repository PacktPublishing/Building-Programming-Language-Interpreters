#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_MESSAGESEQUENCE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_MESSAGESEQUENCE_HPP

#include <memory>
#include <networkprotocoldsl/parser/tree/messagepart.hpp>
#include <vector>

namespace networkprotocoldsl::parser::tree {

struct MessageSequence
    : public std::vector<std::shared_ptr<const MessagePart>> {};

} // namespace networkprotocoldsl::parser::tree

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_MESSAGESEQUENCE_HPP