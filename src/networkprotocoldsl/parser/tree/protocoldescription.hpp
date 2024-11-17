#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_PROTOCOLDESCRIPTION_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_PROTOCOLDESCRIPTION_HPP

#include <networkprotocoldsl/parser/tree/message.hpp>

#include <map>
#include <memory>
#include <string>

namespace networkprotocoldsl::parser::tree {

using ProtocolDescription =
    std::map<std::string, std::shared_ptr<const Message>>;

}

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_PROTOCOLDESCRIPTION_HPP