#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TOKENSEQUENCE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TOKENSEQUENCE_HPP

#include <memory>
#include <networkprotocoldsl/parser/tree/tokenpart.hpp>
#include <vector>

namespace networkprotocoldsl::parser::tree {

struct TokenSequence {
  std::vector<std::shared_ptr<const TokenPart>> tokens;
};

} // namespace networkprotocoldsl::parser::tree

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TOKENSEQUENCE_HPP