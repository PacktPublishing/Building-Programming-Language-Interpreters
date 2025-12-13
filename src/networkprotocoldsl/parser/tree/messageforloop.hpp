#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_MESSAGEFORLOOP_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_MESSAGEFORLOOP_HPP

#include <memory>
#include <networkprotocoldsl/parser/tree/identifierreference.hpp>
#include <networkprotocoldsl/parser/tree/messagesequence.hpp>
#include <optional>
#include <string>

namespace networkprotocoldsl::parser::tree {

struct MessageSequence;
struct MessageForLoop {
  std::shared_ptr<const IdentifierReference> variable;
  std::shared_ptr<const IdentifierReference> collection;
  std::shared_ptr<const MessageSequence> block;
  std::optional<std::string> terminator;
  std::string stringify() const {
    std::string result = "for";
    if (terminator.has_value()) {
      result += " <terminator=\"" + terminator.value() + "\">";
    }
    result += " " + variable->stringify() + " in " + collection->stringify() +
              " {...}";
    return result;
  }
};

} // namespace networkprotocoldsl::parser::tree

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_TREE_MESSAGEFORLOOP_HPP