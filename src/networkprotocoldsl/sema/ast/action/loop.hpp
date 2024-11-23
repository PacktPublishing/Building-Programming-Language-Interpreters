#ifndef INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_ACTION_LOOP_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_ACTION_LOOP_HPP

#include <memory>
#include <string>
#include <vector>

#include <networkprotocoldsl/parser/tree/identifierreference.hpp>
#include <networkprotocoldsl/sema/ast/action.hpp>

namespace networkprotocoldsl::sema::ast::action {

struct Loop {
  std::shared_ptr<const parser::tree::IdentifierReference> list;
  std::shared_ptr<const parser::tree::IdentifierReference> item;
  std::string terminator;
  std::vector<std::shared_ptr<const Action>> actions;
};

} // namespace networkprotocoldsl::sema::ast::action

#endif