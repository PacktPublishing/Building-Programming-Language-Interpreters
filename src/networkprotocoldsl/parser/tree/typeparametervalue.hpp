#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TYPEPARAMETERVALUE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TYPEPARAMETERVALUE_HPP

#include <networkprotocoldsl/parser/tree/booleanliteral.hpp>
#include <networkprotocoldsl/parser/tree/identifierreference.hpp>
#include <networkprotocoldsl/parser/tree/integerliteral.hpp>

#include <variant>

namespace networkprotocoldsl::parser::tree {

class Type; // Forward declaration
using TypeParameterValue =
    std::variant<BooleanLiteral, IntegerLiteral, IdentifierReference, Type>;

} // namespace networkprotocoldsl::parser::tree

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TYPEPARAMETERVALUE_HPP