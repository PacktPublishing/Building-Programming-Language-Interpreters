#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TERMINATOR_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TERMINATOR_HPP

#include <networkprotocoldsl/parser/stringliteral.hpp>

#include <memory>

namespace networkprotocoldsl::parser {

struct Terminator {
  std::shared_ptr<const StringLiteral> value;
};

} // namespace networkprotocoldsl::parser

#endif