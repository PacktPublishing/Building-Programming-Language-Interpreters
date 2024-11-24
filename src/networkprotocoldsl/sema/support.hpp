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
std::vector<ParseStateTraits::TokenIterator::value_type>
unroll_variant(const V &v) {
  auto r = std::vector<ParseStateTraits::TokenIterator::value_type>();
  for (const auto &part : v) {
    std::visit([&](auto &&arg) { r.push_back(arg); }, *part);
  }
  return r;
}

void append_actions(std::vector<ast::Action> &actions,
                    const std::vector<ast::Action> &new_actions) {
  actions.insert(actions.end(), new_actions.cbegin(), new_actions.cend());
}

void append_actions(std::vector<ast::Action> &actions, ast::Action new_action) {
  actions.push_back(new_action);
}

} // namespace networkprotocoldsl::sema

#endif