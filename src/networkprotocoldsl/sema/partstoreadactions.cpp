#include <networkprotocoldsl/sema/partstoreadactions.hpp>

#include <networkprotocoldsl/parser/grammar/traits.hpp>
#include <networkprotocoldsl/parser/support/recursiveparser.hpp>
#include <networkprotocoldsl/parser/tree/identifierreference.hpp>
#include <networkprotocoldsl/parser/tree/messagesequence.hpp>
#include <networkprotocoldsl/parser/tree/stringliteral.hpp>
#include <networkprotocoldsl/parser/tree/tokensequence.hpp>
#include <networkprotocoldsl/sema/ast/action.hpp>
#include <networkprotocoldsl/sema/support.hpp>

#include <memory>
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
    return {std::make_shared<const ast::action::ReadStaticOctets>(lit->value),
            begin, end};
  }
  static void
      partial_match(std::shared_ptr<const parser::tree::IdentifierReference>){};
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end,
        std::shared_ptr<const parser::tree::IdentifierReference> id,
        std::shared_ptr<const parser::tree::StringLiteral> lit) {
    return {std::make_shared<const ast::action::ReadOctetsUntilTerminator>(
                lit->value, id),
            begin, end};
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

class PartSequenceFragmentToReadActions;

class PartSequenceToReadActions
    : public parser::support::RecursiveParser<
          PartSequenceToReadActions, ParseStateTraits,
          parser::grammar::Tracer<PartSequenceToReadActions>> {
public:
  static constexpr const char *name = "PartSequenceToReadActions";
  static PartSequenceFragmentToReadActions *recurse_many() { return nullptr; };
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

class PartSequenceFragmentToReadActions
    : public parser::support::RecursiveParser<
          PartSequenceFragmentToReadActions, ParseStateTraits,
          parser::grammar::Tracer<PartSequenceFragmentToReadActions>> {
public:
  static constexpr const char *name = "PartSequenceFragmentToReadActions";
  static void partial_match(){};
  static void
      partial_match(std::shared_ptr<const parser::tree::TokenSequence>){};

  // Helper to apply escape replacement to ReadOctetsUntilTerminator actions
  static void apply_escape_to_actions(std::vector<ast::Action> &actions,
                                       const parser::tree::EscapeReplacement &escape) {
    ast::action::EscapeInfo escape_info{escape.character, escape.sequence};
    for (auto &action : actions) {
      std::visit([&](auto &a) {
        using T = std::decay_t<decltype(*a)>;
        if constexpr (std::is_same_v<T, ast::action::ReadOctetsUntilTerminator>) {
          // Create a new action with the escape info
          auto new_action = std::make_shared<ast::action::ReadOctetsUntilTerminator>();
          new_action->terminator = a->terminator;
          new_action->identifier = a->identifier;
          new_action->escape = escape_info;
          a = new_action;
        }
      }, action);
    }
  }

  // Helper to process a token sequence with its options (terminator and escape)
  static ParseStateReturn
  process_token_sequence(TokenIterator begin, TokenIterator end,
                         std::shared_ptr<const parser::tree::TokenSequence> token_sequence,
                         const std::string &terminator_str) {
    // Parse the token sequence, adding the terminator to the end
    auto full_sequence = unroll_variant(token_sequence->tokens);
    // Create a StringLiteral for the terminator
    auto terminator_lit = std::make_shared<parser::tree::StringLiteral>(terminator_str);
    full_sequence.push_back(terminator_lit);
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
        std::shared_ptr<const parser::tree::TokenSequence> token_sequence,
        std::shared_ptr<const parser::tree::Terminator> terminator) {
    // We have an external Terminator block - check that token_sequence doesn't have embedded terminator
    if (token_sequence->terminator.has_value()) {
      // Error: conflicting terminator definitions - both embedded terminator option
      // in tokens<terminator="..."> AND explicit terminator { } block.
      // Use one or the other, not both.
      return {std::nullopt, begin, end};
    }
    // Use the external Terminator block's value
    return process_token_sequence(begin, end, token_sequence, terminator->value->value);
  }
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end,
        std::shared_ptr<const parser::tree::TokenSequence> token_sequence,
        EndOfInput) {
    // If the token sequence has an embedded terminator option, use it
    if (token_sequence->terminator.has_value()) {
      return process_token_sequence(begin, end, token_sequence, token_sequence->terminator.value());
    }
    // Otherwise, parse without a terminator
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
  static void
      partial_match(std::shared_ptr<const parser::tree::MessageForLoop>){};
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end,
        std::shared_ptr<const parser::tree::MessageForLoop> loop,
        std::shared_ptr<const parser::tree::Terminator> terminator) {
    auto l = std::make_shared<ast::action::Loop>(
        loop->variable, loop->collection, terminator->value->value);
    auto full_sequence = unroll_variant(*(loop->block));
    auto r = PartSequenceToReadActions::parse(full_sequence.cbegin(),
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

std::optional<std::vector<ast::Action>> parts_to_read_actions(
    const std::shared_ptr<const parser::tree::MessageSequence> &parts) {
  auto full_sequence = unroll_variant(*parts);
  auto r = PartSequenceToReadActions::parse(full_sequence.cbegin(),
                                            full_sequence.cend());
  if (r.node.has_value()) {
    return flatten_actions(r.node.value());
  } else {
    return std::nullopt;
  }
}

} // namespace networkprotocoldsl::sema