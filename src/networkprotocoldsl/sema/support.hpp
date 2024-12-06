#ifndef INCLUDED_NETWORKPROTOCOLDSL_SEMA_SUPPORT_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_SEMA_SUPPORT_HPP

#include <networkprotocoldsl/parser/grammar/traits.hpp>
#include <networkprotocoldsl/parser/support/recursiveparser.hpp>
#include <networkprotocoldsl/sema/ast/action.hpp>

#include <memory>
#include <vector>

namespace networkprotocoldsl::sema {

using ParseStateTraits = parser::support::ParseStateTraits<
    std::vector<parser::grammar::ParseTraits::ParseNode>::const_iterator,
    std::variant<std::vector<ast::Action>, ast::Action>>;

template <typename V>
inline std::vector<ParseStateTraits::TokenIterator::value_type>
unroll_variant(const V &v) {
  auto r = std::vector<ParseStateTraits::TokenIterator::value_type>();
  for (const auto &part : v) {
    std::visit([&](auto &&arg) { r.push_back(arg); }, *part);
  }
  return r;
}

inline void append_actions(std::vector<ast::Action> &actions,
                           const std::vector<ast::Action> &new_actions) {
  actions.insert(actions.end(), new_actions.cbegin(), new_actions.cend());
}

inline void append_actions(std::vector<ast::Action> &actions,
                           ast::Action new_action) {
  actions.push_back(new_action);
}

inline std::vector<ast::Action>
flatten_actions(ParseStateTraits::ParseNode node) {
  std::vector<ast::Action> actions;
  std::visit([&](auto &&t) { append_actions(actions, t); }, node);
  return actions;
}

inline std::vector<ast::Action>
flatten_actions(std::vector<ParseStateTraits::ParseNode> nodes) {
  std::vector<ast::Action> actions;
  for (auto &node : nodes) {
    auto flattened = flatten_actions(node);
    actions.insert(actions.end(), flattened.cbegin(), flattened.cend());
  }
  return actions;
}

} // namespace networkprotocoldsl::sema

#endif