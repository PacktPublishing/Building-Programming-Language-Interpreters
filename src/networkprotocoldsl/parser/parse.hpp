#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_PARSE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_PARSE_HPP

#include <memory>
#include <optional>
#include <vector>

#include <networkprotocoldsl/lexer/token.hpp>
#include <networkprotocoldsl/parser/tree/protocoldescription.hpp>

namespace networkprotocoldsl::parser {

std::optional<std::shared_ptr<const tree::ProtocolDescription>>
parse(const std::vector<lexer::Token> &tokens);

}

#endif