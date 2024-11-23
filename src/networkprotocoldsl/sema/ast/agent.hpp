#ifndef INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_AGENT_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_AGENT_HPP

#include <memory>

#include <networkprotocoldsl/sema/ast/state.hpp>

namespace networkprotocoldsl::sema::ast {

struct Agent {
  std::shared_ptr<const State> initial_state;
};

} // namespace networkprotocoldsl::sema::ast

#endif