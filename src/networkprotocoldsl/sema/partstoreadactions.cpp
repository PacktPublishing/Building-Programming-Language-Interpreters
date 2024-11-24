#include <networkprotocoldsl/sema/partstoreadactions.hpp>

#include <networkprotocoldsl/parser/grammar/traits.hpp>
#include <networkprotocoldsl/parser/support/recursiveparser.hpp>
#include <networkprotocoldsl/parser/tree/identifierreference.hpp>
#include <networkprotocoldsl/parser/tree/messagesequence.hpp>
#include <networkprotocoldsl/parser/tree/stringliteral.hpp>
#include <networkprotocoldsl/parser/tree/tokensequence.hpp>
#include <networkprotocoldsl/sema/ast/action.hpp>

#include <memory>
#include <vector>

namespace networkprotocoldsl::sema {

namespace {

using ParseStateTraits = parser::support::ParseStateTraits<
    std::vector<parser::grammar::ParseTraits::ParseNode>::const_iterator,
    std::variant<std::vector<ast::Action>, ast::Action>>;

template <typename V>
static std::vector<ParseStateTraits::TokenIterator::value_type>
unroll_variant(const V &v) {
  auto r = std::vector<ParseStateTraits::TokenIterator::value_type>();
  for (const auto &part : v) {
    std::visit([&](auto &&arg) { r.push_back(arg); }, *part);
  }
  return r;
}

static void append_actions(std::vector<ast::Action> &actions,
                           const std::vector<ast::Action> &new_actions) {
  actions.insert(actions.end(), new_actions.cbegin(), new_actions.cend());
}

static void append_actions(std::vector<ast::Action> &actions,
                           ast::Action new_action) {
  actions.push_back(new_action);
}

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

    std::vector<ast::Action> actions;
    for (auto &other : others) {
      std::visit([&](auto &&t) { append_actions(actions, t); }, other);
    }
    return {actions, begin, end};
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
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end,
        std::shared_ptr<const parser::tree::TokenSequence> token_sequence,
        std::shared_ptr<const parser::tree::Terminator> terminator) {
    // Parse the token sequence, but add the terminator to the end
    auto full_sequence = unroll_variant(token_sequence->tokens);
    full_sequence.push_back(terminator->value);
    auto r = TokenSequence::parse(full_sequence.cbegin(), full_sequence.cend());
    return {r.node, begin, end};
  }
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end,
        std::shared_ptr<const parser::tree::TokenSequence> token_sequence,
        EndOfInput) {
    // Parse the token sequence
    auto full_sequence = unroll_variant(token_sequence->tokens);
    auto r = TokenSequence::parse(full_sequence.cbegin(), full_sequence.cend());
    return {r.node, begin, end};
  }
  static void
      partial_match(std::shared_ptr<const parser::tree::MessageForLoop>){};
  static ParseStateReturn
  match(TokenIterator begin, TokenIterator end,
        std::shared_ptr<const parser::tree::MessageForLoop> loop,
        std::shared_ptr<const parser::tree::Terminator> terminator) {
    auto l = std::make_shared<ast::action::Loop>(ast::action::Loop{
        loop->variable, loop->collection, terminator->value->value, {}});
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
    std::vector<ast::Action> actions;
    std::visit([&](auto &&t) { append_actions(actions, t); }, r.node.value());
    return actions;
  } else {
    return std::nullopt;
  }
}

} // namespace networkprotocoldsl::sema