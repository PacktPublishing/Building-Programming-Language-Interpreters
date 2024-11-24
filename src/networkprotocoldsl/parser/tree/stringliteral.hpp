#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_STRINGLITERAL_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_STRINGLITERAL_HPP

#include <string>

namespace networkprotocoldsl::parser::tree {

struct StringLiteral {
  std::string value;
  std::string stringify() const { return "\"" + value + "\""; }
};

} // namespace networkprotocoldsl::parser::tree

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_STRINGLITERAL_HPP