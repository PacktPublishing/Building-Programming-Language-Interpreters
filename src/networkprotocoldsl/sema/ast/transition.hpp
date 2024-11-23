#ifndef INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_TRANSITION_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_TRANSITION_HPP

#include <memory>

#include <networkprotocoldsl/sema/ast/message.hpp>

namespace networkprotocoldsl::sema::ast {

struct State; // Forward declaration

struct Transition {
  std::shared_ptr<const Message> message;
  std::shared_ptr<const State> next_state;
};

} // namespace networkprotocoldsl::sema::ast

#endif