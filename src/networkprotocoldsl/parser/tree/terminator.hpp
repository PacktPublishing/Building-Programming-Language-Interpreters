#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TERMINATOR_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TERMINATOR_HPP

#include <networkprotocoldsl/parser/tree/stringliteral.hpp>

#include <memory>

namespace networkprotocoldsl::parser::tree {

struct Terminator {
  std::shared_ptr<const StringLiteral> value;
  std::string stringify() const {
    return "terminator {" + value->stringify() + "}";
  }
};

} // namespace networkprotocoldsl::parser::tree

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TERMINATOR_HPP