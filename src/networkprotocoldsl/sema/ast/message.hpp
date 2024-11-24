#ifndef INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_MESSAGE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_MESSAGE_HPP

#include <memory>
#include <vector>

#include <networkprotocoldsl/parser/tree/message.hpp>
#include <networkprotocoldsl/sema/ast/action.hpp>

namespace networkprotocoldsl::sema::ast {

struct Message {
  std::shared_ptr<const parser::tree::Message> parsed;
  std::vector<std::shared_ptr<const Action>> actions;
};

} // namespace networkprotocoldsl::sema::ast

#endif