#ifndef INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_ACTION_READ_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_ACTION_READ_HPP

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <networkprotocoldsl/parser/tree/identifierreference.hpp>

namespace networkprotocoldsl::sema::ast::action {

struct ReadStaticOctets {
  std::string octets;
};

// Escape replacement info for ReadOctetsUntilTerminator
struct EscapeInfo {
  std::string character;  // what to insert in the captured value (e.g., "\n")
  std::string sequence;   // what appears on the wire (e.g., "\r\n ")
};

struct ReadOctetsUntilTerminator {
  std::string terminator;
  std::shared_ptr<const parser::tree::IdentifierReference> identifier;
  // Optional escape replacement - if present:
  // - when parsing: replace escape_sequence with escape_char in captured value
  // - when serializing: replace escape_char with escape_sequence in output
  std::optional<EscapeInfo> escape;
};

} // namespace networkprotocoldsl::sema::ast::action

#endif