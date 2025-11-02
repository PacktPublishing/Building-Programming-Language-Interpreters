#ifndef INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_TRANSITION_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_TRANSITION_HPP

#include <memory>

#include <networkprotocoldsl/parser/tree/message.hpp>
#include <networkprotocoldsl/sema/ast/action.hpp>

namespace networkprotocoldsl::sema::ast {

struct AbstractTransition {
  std::shared_ptr<const parser::tree::MessageData> data;
  std::vector<Action> actions;
  AbstractTransition(
      const std::shared_ptr<const parser::tree::MessageData> &d,
      const std::vector<Action> &a)
      : data(d), actions(a) {}
};
struct ReadTransition : public AbstractTransition {
  using AbstractTransition::AbstractTransition;
};
struct WriteTransition : public AbstractTransition {
  using AbstractTransition::AbstractTransition;
};
using Transition = std::variant<std::shared_ptr<const ReadTransition>,
                                std::shared_ptr<const WriteTransition>>;

} // namespace networkprotocoldsl::sema::ast

#endif