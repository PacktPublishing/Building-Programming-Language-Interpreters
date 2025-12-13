#ifndef INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_ACTION_WRITE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_ACTION_WRITE_HPP

#include <memory>
#include <optional>
#include <string>

#include <networkprotocoldsl/parser/tree/identifierreference.hpp>
#include <networkprotocoldsl/sema/ast/action/read.hpp>

namespace networkprotocoldsl::sema::ast::action {

struct WriteFromIdentifier {
  std::shared_ptr<const parser::tree::IdentifierReference> identifier;
  // Optional escape replacement - if present:
  // when serializing: replace escape_char with escape_sequence in output
  std::optional<EscapeInfo> escape;
};

struct WriteStaticOctets {
  std::string octets;
};

} // namespace networkprotocoldsl::sema::ast::action

#endif