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

struct ReadOctetsUntilTerminator {
  std::string terminator;
  std::shared_ptr<const parser::tree::IdentifierReference> identifier;
  // Optional escape sequence - if present and found before terminator,
  // the escape is removed and reading continues (for HTTP header continuation)
  std::optional<std::string> escape;
};

} // namespace networkprotocoldsl::sema::ast::action

#endif