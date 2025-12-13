#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TOKENSEQUENCE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TOKENSEQUENCE_HPP

#include <memory>
#include <networkprotocoldsl/parser/tree/tokenpart.hpp>
#include <networkprotocoldsl/parser/tree/tokensequenceoptions.hpp>
#include <optional>
#include <string>
#include <vector>

namespace networkprotocoldsl::parser::tree {

struct TokenSequence {
  std::vector<std::shared_ptr<const TokenPart>> tokens;
  std::optional<std::string> terminator;
  std::optional<EscapeReplacement> escape;
  std::string stringify() const {
    std::string result = "tokens";
    if (terminator.has_value() || escape.has_value()) {
      result += " <";
      if (terminator.has_value()) {
        result += "terminator=\"" + terminator.value() + "\"";
        if (escape.has_value()) {
          result += ", ";
        }
      }
      if (escape.has_value()) {
        result += "escape=replace<\"" + escape.value().character + "\", \"" + escape.value().sequence + "\">";
      }
      result += ">";
    }
    result += " {";
    for (const auto &part : tokens) {
      result += std::visit([](auto &&arg) { return arg->stringify(); }, *part);
    }
    result += "}";
    return result;
  }
};

} // namespace networkprotocoldsl::parser::tree

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TOKENSEQUENCE_HPP