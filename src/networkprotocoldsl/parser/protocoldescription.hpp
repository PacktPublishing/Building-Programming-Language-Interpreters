#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_PROTOCOLDESCRIPTION_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_PROTOCOLDESCRIPTION_HPP

#include <networkprotocoldsl/parser/message.hpp>

#include <map>
#include <memory>
#include <string>

namespace networkprotocoldsl::parser {

using ProtocolDescription =
    std::map<std::string, std::shared_ptr<const Message>>;

}

#endif