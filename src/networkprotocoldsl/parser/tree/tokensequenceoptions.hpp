#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TOKENSEQUENCEOPTIONS_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TOKENSEQUENCEOPTIONS_HPP

#include <map>
#include <optional>
#include <string>
#include <utility>
#include <variant>

namespace networkprotocoldsl::parser::tree {

// Escape replacement: replace<"char", "sequence">
// - character: the character to represent in the captured value (e.g., "\n")
// - sequence: the escape sequence on the wire (e.g., "\r\n ")
struct EscapeReplacement {
  std::string character;  // what appears in the captured/serialized value
  std::string sequence;   // what appears on the wire
};

// A token sequence option value can be a simple string or an escape replacement
using TokenSequenceOptionValue = std::variant<std::string, EscapeReplacement>;

// A key-value pair for token sequence options (e.g., terminator="\r\n")
using TokenSequenceOptionPair = std::pair<std::string, TokenSequenceOptionValue>;

// A map of option names to their values
using TokenSequenceOptionsMap = std::map<std::string, TokenSequenceOptionValue>;

} // namespace networkprotocoldsl::parser::tree

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TOKENSEQUENCEOPTIONS_HPP
