#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TOKENSEQUENCE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TOKENSEQUENCE_HPP

#include <memory>
#include <networkprotocoldsl/parser/tokenpart.hpp>
#include <vector>

namespace networkprotocoldsl::parser {

struct TokenSequence {
  std::vector<std::shared_ptr<const TokenPart>> tokens;
};

} // namespace networkprotocoldsl::parser

#endif