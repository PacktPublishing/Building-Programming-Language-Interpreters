#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TOKENSEQUENCEOPTIONS_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TOKENSEQUENCEOPTIONS_HPP

#include <map>
#include <string>
#include <utility>

namespace networkprotocoldsl::parser::tree {

// A key-value pair for token sequence options (e.g., terminator="\r\n")
using TokenSequenceOptionPair = std::pair<std::string, std::string>;

// A map of option names to their string values
using TokenSequenceOptionsMap = std::map<std::string, std::string>;

} // namespace networkprotocoldsl::parser::tree

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_TOKENSEQUENCEOPTIONS_HPP
