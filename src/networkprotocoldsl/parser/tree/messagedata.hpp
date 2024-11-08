#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_MESSAGEDATA_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_MESSAGEDATA_HPP

#include <networkprotocoldsl/parser/tree/type.hpp>

#include <map>
#include <memory>
#include <string>

namespace networkprotocoldsl::parser::tree {

using MessageDataPair = std::pair<std::string, std::shared_ptr<const Type>>;

using MessageData = std::map<std::string, std::shared_ptr<const Type>>;

} // namespace networkprotocoldsl::parser::tree

#endif