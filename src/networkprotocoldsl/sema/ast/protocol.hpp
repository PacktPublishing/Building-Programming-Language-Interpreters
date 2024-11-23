#ifndef INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_PROTOCOL_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_PROTOCOL_HPP

#include <memory>

#include <networkprotocoldsl/sema/ast/agent.hpp>

namespace networkprotocoldsl::sema::ast {

struct Protocol {
  std::shared_ptr<const Agent> client;
  std::shared_ptr<const Agent> server;
};

} // namespace networkprotocoldsl::sema::ast

#endif