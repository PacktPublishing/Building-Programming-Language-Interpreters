#ifndef INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_ACTION_WRITE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_ACTION_WRITE_HPP

#include <memory>
#include <string>

#include <networkprotocoldsl/parser/tree/identifierreference.hpp>

namespace networkprotocoldsl::sema::ast::action {

struct WriteFromIdentifier {
  std::shared_ptr<const parser::tree::IdentifierReference> identifier;
};

struct WriteStaticOctets {
  std::string octets;
};

} // namespace networkprotocoldsl::sema::ast::action

#endif