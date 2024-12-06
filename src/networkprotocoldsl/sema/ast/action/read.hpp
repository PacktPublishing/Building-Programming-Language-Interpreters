#ifndef INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_ACTION_READ_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_ACTION_READ_HPP

#include <memory>
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
};

} // namespace networkprotocoldsl::sema::ast::action

#endif