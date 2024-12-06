#ifndef INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_STATE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_STATE_HPP

#include <map>
#include <memory>

#include <networkprotocoldsl/sema/ast/transition.hpp>

namespace networkprotocoldsl::sema::ast {

struct State {
  std::map<std::string, std::pair<Transition, std::string>> transitions;
};

} // namespace networkprotocoldsl::sema::ast

#endif