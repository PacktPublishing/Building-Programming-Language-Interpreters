#include <networkprotocoldsl/parser/tree/messagesequence.hpp>
#include <networkprotocoldsl/parser/tree/tokensequenceoptions.hpp>
#include <networkprotocoldsl/sema/ast/action.hpp>
#include <networkprotocoldsl/sema/partstowriteactions.hpp>
#include <networkprotocoldsl/sema/support.hpp>

#include <memory>
#include <optional>
#include <vector>

namespace networkprotocoldsl::sema {

namespace {

class IndividualTokenAction
    : public parser::support::RecursiveParser<
          IndividualTokenAction, ParseStateTraits,
          parser::grammar::Tracer<IndividualTokenAction>> {
public:
  static constexpr const char *name = "IndividualTokenAction";
  static void partial_match(){};
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end,
        std::shared_ptr<const parser::tree::StringLiteral> lit) {
    return {std::make_shared<const ast::action::WriteStaticOctets>(lit->value),
            begin, end};
  }
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end,
        std::shared_ptr<const parser::tree::IdentifierReference> id) {
    return {std::make_shared<const ast::action::WriteFromIdentifier>(id), begin,
            end};
  }
};

class TokenSequence : public parser::support::RecursiveParser<
                          TokenSequence, ParseStateTraits,
                          parser::grammar::Tracer<TokenSequence>> {
public:
  static constexpr const char *name = "TokenSequence";
  static IndividualTokenAction *recurse_many() { return nullptr; };
  static void partial_match(std::vector<ast::Action>){};
  static ParseStateReturn match(TokenIterator begin, TokenIterator end,
                                std::vector<ast::Action> actions, EndOfInput) {
    return {actions, begin, end};
  }
};

class PartSequenceFragmentToWriteActions;

class PartSequenceToWriteActions
    : public parser::support::RecursiveParser<
          PartSequenceToWriteActions, ParseStateTraits,
          parser::grammar::Tracer<PartSequenceToWriteActions>> {
public:
  static constexpr const char *name = "PartSequenceToWriteActions";
  static PartSequenceFragmentToWriteActions *recurse_many() { return nullptr; };
  static void partial_match(std::vector<ast::Action>){};
  static void partial_match(std::vector<std::vector<ast::Action>>){};
  static void partial_match(std::vector<ParseNode>){};
  static ParseStateReturn match(TokenIterator begin, TokenIterator end,
                                std::vector<ast::Action> actions, EndOfInput) {
    return {actions, begin, end};
  }
  static ParseStateReturn match(TokenIterator begin, TokenIterator end,
                                std::vector<std::vector<ast::Action>> actions,
                                EndOfInput) {
    std::vector<ast::Action> result;
    for (const auto &a : actions) {
      result.insert(result.end(), a.cbegin(), a.cend());
    }
    return {result, begin, end};
  }
  static ParseStateReturn match(TokenIterator begin, TokenIterator end,
                                std::vector<ParseNode> others, EndOfInput) {
    return {flatten_actions(others), begin, end};
  }
};

class PartSequenceFragmentToWriteActions
    : public parser::support::RecursiveParser<
          PartSequenceFragmentToWriteActions, ParseStateTraits,
          parser::grammar::Tracer<PartSequenceFragmentToWriteActions>> {
public:
  static constexpr const char *name = "PartSequenceFragmentToWriteActions";
  static void partial_match(){};

  // Helper to apply escape replacement to WriteFromIdentifier actions
  static void apply_escape_to_actions(std::vector<ast::Action> &actions,
                                       const parser::tree::EscapeReplacement &escape) {
    ast::action::EscapeInfo escape_info{escape.character, escape.sequence};
    for (auto &action : actions) {
      std::visit([&](auto &a) {
        using T = std::decay_t<decltype(*a)>;
        if constexpr (std::is_same_v<T, ast::action::WriteFromIdentifier>) {
          // Create a new action with the escape info
          auto new_action = std::make_shared<ast::action::WriteFromIdentifier>();
          new_action->identifier = a->identifier;
          new_action->escape = escape_info;
          a = new_action;
        }
      }, action);
    }
  }

  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end,
        std::shared_ptr<const parser::tree::TokenSequence> token_sequence) {
    // Parse the token sequence, but add the terminator to the end
    auto full_sequence = unroll_variant(token_sequence->tokens);
    auto r = TokenSequence::parse(full_sequence.cbegin(), full_sequence.cend());
    
    // Apply escape replacement if present
    if (r.node.has_value() && token_sequence->escape.has_value()) {
      auto actions = std::get<std::vector<ast::Action>>(r.node.value());
      apply_escape_to_actions(actions, token_sequence->escape.value());
      return {actions, begin, end};
    }
    return {r.node, begin, end};
  }
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end,
        std::shared_ptr<const parser::tree::Terminator> terminator) {
    // Parse the token sequence, but add the terminator to the end
    auto a = std::make_shared<const ast::action::WriteStaticOctets>(
        terminator->value->value);
    return {a, begin, end};
  }
  static void
      partial_match(std::shared_ptr<const parser::tree::MessageForLoop>){};
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end,
        std::shared_ptr<const parser::tree::MessageForLoop> loop,
        std::shared_ptr<const parser::tree::Terminator> terminator) {
    auto l = std::make_shared<ast::action::Loop>(
        loop->variable, loop->collection, terminator->value->value);
    auto full_sequence = unroll_variant(*(loop->block));
    auto r = PartSequenceToWriteActions::parse(full_sequence.cbegin(),
                                               full_sequence.cend());
    if (r.node.has_value()) {
      std::visit([&](auto &&t) { append_actions(l->actions, t); },
                 r.node.value());
      return {l, begin, end};
    } else {
      return {std::nullopt, begin, end};
    }
  }
};

} // anonymous namespace

std::optional<std::vector<ast::Action>> parts_to_write_actions(
    const std::shared_ptr<const parser::tree::MessageSequence> &parts) {
  auto full_sequence = unroll_variant(*parts);
  auto r = PartSequenceToWriteActions::parse(full_sequence.cbegin(),
                                             full_sequence.cend());
  if (r.node.has_value()) {
    return flatten_actions(r.node.value());
  } else {
    return std::nullopt;
  }
}

} // namespace networkprotocoldsl::sema
