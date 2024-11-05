#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TYPEPARAMETERVALUE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TYPEPARAMETERVALUE_HPP

#include <networkprotocoldsl/parser/booleanliteral.hpp>
#include <networkprotocoldsl/parser/identifierreference.hpp>
#include <networkprotocoldsl/parser/integerliteral.hpp>

#include <variant>

namespace networkprotocoldsl::parser {

class Type; // forward declaration

using TypeParameterValue =
    std::variant<BooleanLiteral, IntegerLiteral, IdentifierReference, Type>;

} // namespace networkprotocoldsl::parser

#endif