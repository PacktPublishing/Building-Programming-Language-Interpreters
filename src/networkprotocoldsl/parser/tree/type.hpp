#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TYPE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TYPE_HPP

#include <memory>
#include <networkprotocoldsl/parser/tree/identifierreference.hpp>
#include <networkprotocoldsl/parser/tree/typeparametermap.hpp>

namespace networkprotocoldsl::parser::tree {

struct Type {
  std::shared_ptr<const IdentifierReference> name;
  std::shared_ptr<const TypeParameterMap> parameters;
};

} // namespace networkprotocoldsl::parser::tree

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TYPE_HPP