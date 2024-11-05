#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TOKENPART_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TOKENPART_HPP

#include <networkprotocoldsl/parser/identifierreference.hpp>
#include <networkprotocoldsl/parser/stringliteral.hpp>

#include <memory>
#include <variant>

namespace networkprotocoldsl::parser {

using TokenPart = std::variant<std::shared_ptr<const StringLiteral>,
                               std::shared_ptr<const IdentifierReference>>;

}

#endif