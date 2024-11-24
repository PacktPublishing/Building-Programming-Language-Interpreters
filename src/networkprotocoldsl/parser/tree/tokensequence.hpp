#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TOKENSEQUENCE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TOKENSEQUENCE_HPP

#include <memory>
#include <networkprotocoldsl/parser/tree/tokenpart.hpp>
#include <vector>

namespace networkprotocoldsl::parser::tree {

struct TokenSequence {
  std::vector<std::shared_ptr<const TokenPart>> tokens;
  std::string stringify() const {
    std::string result = "tokens {";
    for (const auto &part : tokens) {
      result += std::visit([](auto &&arg) { return arg->stringify(); }, *part);
    }
    result += "}";
    return result;
  }
};

} // namespace networkprotocoldsl::parser::tree

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TOKENSEQUENCE_HPP