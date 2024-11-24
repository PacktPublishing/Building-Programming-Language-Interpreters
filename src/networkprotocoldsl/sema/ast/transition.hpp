#ifndef INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_TRANSITION_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_TRANSITION_HPP

#include <memory>

#include <networkprotocoldsl/sema/ast/message.hpp>

namespace networkprotocoldsl::sema::ast {

struct AbstractTransition {
  std::shared_ptr<const parser::tree::MessageData> data;
  std::vector<Action> actions;
};

struct ReadTransition : public AbstractTransition {};
struct WriteTransition : public AbstractTransition {};

using Transition = std::variant<std::shared_ptr<const ReadTransition>,
                                std::shared_ptr<const WriteTransition>>;

} // namespace networkprotocoldsl::sema::ast

#endif