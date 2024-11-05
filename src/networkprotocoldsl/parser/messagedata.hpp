#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_MESSAGEDATA_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_MESSAGEDATA_HPP

#include <networkprotocoldsl/parser/type.hpp>

#include <map>
#include <memory>
#include <string>

namespace networkprotocoldsl::parser {

using MessageData = std::map<std::string, std::shared_ptr<const Type>>;

}

#endif