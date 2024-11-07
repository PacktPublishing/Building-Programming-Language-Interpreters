#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TOKENPART_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TOKENPART_HPP

#include <networkprotocoldsl/parser/tree/identifierreference.hpp>
#include <networkprotocoldsl/parser/tree/stringliteral.hpp>

#include <memory>
#include <variant>

namespace networkprotocoldsl::parser::tree {

using TokenPart = std::variant<std::shared_ptr<const StringLiteral>,
                               std::shared_ptr<const IdentifierReference>>;

}

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TOKENPART_HPP