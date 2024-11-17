#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TYPEPARAMETERVALUE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TYPEPARAMETERVALUE_HPP

#include <networkprotocoldsl/parser/tree/booleanliteral.hpp>
#include <networkprotocoldsl/parser/tree/identifierreference.hpp>
#include <networkprotocoldsl/parser/tree/integerliteral.hpp>
#include <networkprotocoldsl/parser/tree/stringliteral.hpp>

#include <memory>
#include <variant>

namespace networkprotocoldsl::parser::tree {

class Type; // Forward declaration
using TypeParameterValue = std::variant<std::shared_ptr<const BooleanLiteral>,
                                        std::shared_ptr<const IntegerLiteral>,
                                        std::shared_ptr<const StringLiteral>,
                                        std::shared_ptr<const Type>>;

} // namespace networkprotocoldsl::parser::tree

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TYPEPARAMETERVALUE_HPP