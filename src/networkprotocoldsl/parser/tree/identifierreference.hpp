#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_IDENTIFIERREFERENCE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_IDENTIFIERREFERENCE_HPP

#include <string>

namespace networkprotocoldsl::parser::tree {

struct IdentifierReference {
  std::string name;
};

} // namespace networkprotocoldsl::tree

#endif