#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_IDENTIFIERREFERENCE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_IDENTIFIERREFERENCE_HPP

#include <memory>
#include <optional>
#include <string>

namespace networkprotocoldsl::parser::tree {

struct IdentifierReference {
  std::string name;
  std::optional<std::shared_ptr<const IdentifierReference>> member;
};

} // namespace networkprotocoldsl::parser::tree

#endif