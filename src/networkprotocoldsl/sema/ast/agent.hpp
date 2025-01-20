#ifndef INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_AGENT_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_AGENT_HPP

#include <memory>
#include <unordered_map>

#include <networkprotocoldsl/sema/ast/state.hpp>

namespace networkprotocoldsl::sema::ast {

struct Agent {
  std::unordered_map<std::string, std::shared_ptr<const State>> states;
};

} // namespace networkprotocoldsl::sema::ast

#endif