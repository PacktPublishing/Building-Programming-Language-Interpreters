#include <functional>
#include <networkprotocoldsl/generate.hpp>
#include <networkprotocoldsl/operation.hpp>
#include <networkprotocoldsl/sema/ast/action.hpp>

namespace {

enum class TransitionType { Read, Write };

}

namespace networkprotocoldsl {
namespace generate {
using namespace networkprotocoldsl::operation;

using ExtractTypeClosure =
    std::function<std::optional<std::shared_ptr<const parser::tree::Type>>(
        const std::string &)>;
using GenerateOpTreeValueFromType = std::function<std::optional<OpTreeNode>(
    const std::shared_ptr<const parser::tree::Type> &)>;

template <typename TRANSITION>
static std::optional<std::vector<OpTreeNode>>
actions_to_optreenodes(const TRANSITION &transition,
                       const std::vector<sema::ast::Action> &actions,
                       const ExtractTypeClosure &get_type);

static std::optional<ExtractTypeClosure>
create_get_type_wrapper(const ExtractTypeClosure &get_type,
                        const std::string &variable,
                        const std::string &collection) {
  auto maybe_type = get_type(collection);
  if (!maybe_type)
    return std::nullopt;
  auto params = maybe_type.value()->parameters;
  auto it = params->find("element_type");
  if (it == params->end())
    return std::nullopt;
  auto element_type = it->second;
  return [=](const std::string &name)
             -> std::optional<std::shared_ptr<const parser::tree::Type>> {
    if (name == variable) {
      if (std::holds_alternative<std::shared_ptr<const parser::tree::Type>>(
              element_type)) {
        return std::get<std::shared_ptr<const parser::tree::Type>>(
            element_type);
      } else {
        return std::nullopt;
      }
    }
    return get_type(name);
  };
}

static std::optional<std::vector<std::string>> extract_argument_names(
    const std::shared_ptr<const parser::tree::MessageData> &data) {
  if (!data)
    return std::nullopt;
  std::vector<std::string> argument_names;
  for (const auto &arg : *data) {
    argument_names.push_back(arg.first);
  }
  return argument_names;
}

static std::optional<std::shared_ptr<const parser::tree::Type>>
extract_type(const std::shared_ptr<const parser::tree::MessageData> &data,
             const std::string &name) {
  auto it = data->find(name);
  if (it != data->end()) {
    return it->second;
  }
  return std::nullopt;
}

static std::optional<
    std::tuple<OpTreeNode, const std::shared_ptr<const parser::tree::Type>>>
recursive_variable_access(
    const std::shared_ptr<const parser::tree::Type> &type,
    const std::shared_ptr<const parser::tree::IdentifierReference> &identifier,
    OpTreeNode current_access,
    std::optional<GenerateOpTreeValueFromType> value_to_write) {
  auto it = type->parameters->find(identifier->name);
  if (it == type->parameters->end()) {
    return std::nullopt;
  }
  auto member_type =
      std::get<std::shared_ptr<const parser::tree::Type>>(it->second);
  if (!member_type) {
    return std::nullopt;
  }

  if (!identifier->member.has_value()) {
    if (value_to_write.has_value()) {
      auto value_access = value_to_write.value()(member_type);
      if (!value_access) {
        return std::nullopt;
      }
      auto last_access = OpTreeNode{DictionarySet(identifier->name),
                                    {current_access, value_access.value()}};
      return std::make_tuple(last_access, member_type);
    } else {
      auto last_access =
          OpTreeNode{DictionaryGet(identifier->name), {current_access}};
      return std::make_tuple(last_access, member_type);
    }
  } else if (member_type->name->name == "tuple") {
    auto outer_access =
        OpTreeNode{DictionaryGet(identifier->name), {current_access}};
    return recursive_variable_access(member_type, identifier->member.value(),
                                     outer_access, value_to_write);
  } else {
    return std::nullopt;
  }
}

static std::optional<OpTreeNode>
write_octets_from_value(const std::shared_ptr<const parser::tree::Type> &type,
                        OpTreeNode value) {
  if (type->name->name == "int") {
    return OpTreeNode{WriteOctets{}, {{IntToAscii{}, {value}}}};
  } else if (type->name->name == "str") {
    return OpTreeNode{WriteOctets{}, {value}};
  }
  return std::nullopt;
}

static std::optional<OpTreeNode>
read_value_from_octets(const std::shared_ptr<const parser::tree::Type> &type,
                       const std::string &terminator) {
  if (type->name->name == "int") {
    return OpTreeNode{ReadIntFromAscii{},
                      {{ReadOctetsUntilTerminator(terminator), {}}}};
  } else if (type->name->name == "str") {
    return OpTreeNode{ReadOctetsUntilTerminator(terminator), {}};
  }
  return std::nullopt;
}

static std::optional<OpTreeNode> visit_action(
    const std::shared_ptr<const sema::ast::ReadTransition> &transition,
    const ExtractTypeClosure &get_type,
    const std::shared_ptr<const sema::ast::action::ReadOctetsUntilTerminator>
        &action) {
  auto &outermost_identifier = action->identifier->name;
  auto &identifier = outermost_identifier;
  auto maybe_type = get_type(identifier);
  if (!maybe_type)
    return std::nullopt;
  auto member = action->identifier->member;
  if (!member.has_value()) {
    auto maybe_optreenode =
        read_value_from_octets(maybe_type.value(), action->terminator);
    if (!maybe_optreenode)
      return std::nullopt;
    return OpTreeNode{LexicalPadSet(identifier), {*maybe_optreenode}};
  } else {
    OpTreeNode variable_access = OpTreeNode{LexicalPadGet(identifier), {}};
    auto maybe_access = recursive_variable_access(
        maybe_type.value(), member.value(), variable_access,
        [&](const std::shared_ptr<const parser::tree::Type> &type) {
          return read_value_from_octets(type, action->terminator);
        });
    if (!maybe_access)
      return std::nullopt;
    auto [last_access, last_type] = *maybe_access;
    return OpTreeNode{LexicalPadSet(identifier), {last_access}};
  }
  return std::nullopt;
}

static std::optional<OpTreeNode> visit_action(
    const std::shared_ptr<const sema::ast::ReadTransition> &transition,
    const ExtractTypeClosure &get_type,
    const std::shared_ptr<const sema::ast::action::ReadStaticOctets> &action) {
  return OpTreeNode{ReadStaticOctets(action->octets), {}};
}

static std::optional<OpTreeNode> visit_action(
    const std::shared_ptr<const sema::ast::WriteTransition> &transition,
    const ExtractTypeClosure &get_type,
    const std::shared_ptr<const sema::ast::action::WriteFromIdentifier>
        &action) {
  auto &identifier = action->identifier;
  auto maybe_type = get_type(identifier->name);
  if (!maybe_type)
    return std::nullopt;

  OpTreeNode variable_access = OpTreeNode{LexicalPadGet(identifier->name), {}};
  auto member = identifier->member;
  if (member.has_value()) {
    auto maybe_access = recursive_variable_access(
        maybe_type.value(), member.value(), variable_access, std::nullopt);
    if (!maybe_access)
      return std::nullopt;
    auto [last_access, last_type] = *maybe_access;
    return write_octets_from_value(last_type, last_access);
  } else {
    return write_octets_from_value(maybe_type.value(), variable_access);
  }
}

static std::optional<OpTreeNode> visit_action(
    const std::shared_ptr<const sema::ast::WriteTransition> &transition,
    const ExtractTypeClosure &get_type,
    const std::shared_ptr<const sema::ast::action::WriteStaticOctets> &action) {
  return OpTreeNode{WriteStaticOctets(action->octets), {}};
}

static std::optional<OpTreeNode> visit_action(
    const std::shared_ptr<const sema::ast::WriteTransition> &transition,
    const ExtractTypeClosure &get_type,
    const std::shared_ptr<const sema::ast::action::Loop> &action) {
  auto &variable = action->variable->name;
  auto &collection = action->collection->name;
  auto get_type_wrapper =
      create_get_type_wrapper(get_type, variable, collection);
  if (!get_type_wrapper)
    return std::nullopt;
  auto maybe_ops =
      actions_to_optreenodes(transition, action->actions, *get_type_wrapper);
  if (!maybe_ops)
    return std::nullopt;
  auto loop_optree =
      std::make_shared<OpTree>(OpTreeNode{OpSequence{}, *maybe_ops});
  auto static_callable = StaticCallable(loop_optree, {variable}, true);
  return OpTreeNode{
      OpSequence{},
      {
          {
              FunctionCallForEach{true},
              {{static_callable, {}}, {LexicalPadGet(collection), {}}},
          },
          {WriteStaticOctets(action->terminator), {}},
      }};
}

static std::optional<OpTreeNode>
initialize_loop_variable(const ExtractTypeClosure &get_type,
                         const std::string &collection,
                         const std::string &variable) {
  auto maybe_type = get_type(collection);
  if (!maybe_type)
    return std::nullopt;
  if (maybe_type.value()->name->name == "array") {
    auto params = maybe_type.value()->parameters;
    auto it = params->find("element_type");
    if (it == params->end())
      return std::nullopt;
    if (std::holds_alternative<std::shared_ptr<const parser::tree::Type>>(
            it->second) &&
        std::get<std::shared_ptr<const parser::tree::Type>>(it->second)
                ->name->name == "tuple") {
      return OpTreeNode{LexicalPadInitialize(variable),
                        {{DictionaryInitialize{}, {}}}};
    }
  }
  return std::nullopt;
}

static std::optional<OpTreeNode>
visit_action(const std::shared_ptr<const sema::ast::ReadTransition> &transition,
             const ExtractTypeClosure &get_type,
             const std::shared_ptr<const sema::ast::action::Loop> &action) {
  auto &variable = action->variable->name;
  auto &collection = action->collection->name;
  auto get_type_wrapper =
      create_get_type_wrapper(get_type, variable, collection);
  if (!get_type_wrapper)
    return std::nullopt;
  auto maybe_ops =
      actions_to_optreenodes(transition, action->actions, *get_type_wrapper);
  if (!maybe_ops)
    return std::nullopt;
  auto maybe_initialize =
      initialize_loop_variable(*get_type_wrapper, collection, variable);
  if (!maybe_initialize)
    return std::nullopt;
  auto loop_optree = std::make_shared<OpTree>(
      OpTree(OpTreeNode{OpSequence{},
                        {{
                            *maybe_initialize,
                            {OpSequence{}, *maybe_ops},
                            {TerminateListIfReadAhead{action->terminator}, {}},
                            {LexicalPadGet(variable), {}},
                        }}})); // Fixed unbalanced parenthesis here
  auto static_callable = StaticCallable(loop_optree, {}, true);
  return OpTreeNode{LexicalPadSet(collection),
                    {{GenerateList{}, {{static_callable, {}}}}}};
}

static std::optional<OpTreeNode> visit_action(auto &, auto &, auto &) {
  // fallback for unknown action
  return std::nullopt;
}

template <typename TRANSITION>
static std::optional<std::vector<OpTreeNode>>
actions_to_optreenodes(const TRANSITION &transition,
                       const std::vector<sema::ast::Action> &actions,
                       const ExtractTypeClosure &get_type) {
  std::vector<OpTreeNode> operations;
  for (const auto &action : actions) {
    auto maybe_op = std::visit(
        [&](const auto &a) { return visit_action(transition, get_type, a); },
        action);
    if (!maybe_op)
      return std::nullopt;
    operations.push_back(maybe_op.value());
  }
  return operations;
}

static std::optional<std::shared_ptr<const OpTree>> generate_transition_optree(
    const std::shared_ptr<const sema::ast::WriteTransition> &transition) {
  if (!transition)
    return std::nullopt;
  auto get_type = [&](const std::string &name) {
    return extract_type(transition->data, name);
  };
  auto maybe_ops =
      actions_to_optreenodes(transition, transition->actions, get_type);
  if (!maybe_ops)
    return std::nullopt;
  return std::make_shared<OpTree>(
      OpTree(OpTreeNode{OpSequence{},
                        {{OpSequence{}, *maybe_ops},
                         {DynamicList{}, {{LexicalPadAsDict{}, {}}}}}}));
}

static std::optional<std::shared_ptr<const OpTree>> generate_transition_optree(
    const std::shared_ptr<const sema::ast::ReadTransition> &transition) {
  if (!transition)
    return std::nullopt;
  auto get_type = [&](const std::string &name) {
    return extract_type(transition->data, name);
  };
  auto maybe_ops =
      actions_to_optreenodes(transition, transition->actions, get_type);
  if (!maybe_ops)
    return std::nullopt;

  std::vector<OpTreeNode> init_ops;
  auto argument_names = extract_argument_names(transition->data);
  if (argument_names) {
    for (const auto &arg : *argument_names) {
      auto maybe_type = get_type(arg);
      if (!maybe_type)
        return std::nullopt;
      init_ops.push_back(OpTreeNode{LexicalPadInitialize(arg),
                                    {OpTreeNode{Int32Literal{0}, {}}}});
    }
  }

  return std::make_shared<OpTree>(
      OpTree(OpTreeNode{OpSequence{},
                        {{OpSequence{}, init_ops},
                         {OpSequence{}, *maybe_ops},
                         {DynamicList{}, {{LexicalPadAsDict{}, {}}}}}}));
}

static std::optional<StateMachineOperation::TransitionInfo>
generate_transition_info(
    const std::shared_ptr<const sema::ast::WriteTransition> &transition,
    const std::string &target_state) {
  auto optree = generate_transition_optree(transition);
  if (!optree)
    return std::nullopt;
  auto argument_names = extract_argument_names(transition->data);
  if (!argument_names)
    return std::nullopt;
  return StateMachineOperation::TransitionInfo{*optree, *argument_names,
                                               target_state};
}

static std::optional<StateMachineOperation::TransitionInfo>
generate_transition_info(
    const std::shared_ptr<const sema::ast::ReadTransition> &transition,
    const std::string &target_state) {
  auto optree = generate_transition_optree(transition);
  if (!optree)
    return std::nullopt;
  return StateMachineOperation::TransitionInfo{*optree, {}, target_state};
}

static std::optional<
    std::pair<TransitionType, StateMachineOperation::TransitionInfo>>
visit_transition(
    const std::shared_ptr<const sema::ast::WriteTransition> &transition,
    const std::string &target_state) {
  auto maybe_info = generate_transition_info(transition, target_state);
  if (!maybe_info)
    return std::nullopt;
  return {{TransitionType::Write, *maybe_info}};
}

static std::optional<
    std::pair<TransitionType, StateMachineOperation::TransitionInfo>>
visit_transition(
    const std::shared_ptr<const sema::ast::ReadTransition> &transition,
    const std::string &target_state) {
  auto maybe_info = generate_transition_info(transition, target_state);
  if (!maybe_info)
    return std::nullopt;
  return {{TransitionType::Read, *maybe_info}};
}

static std::optional<operation::TransitionLookahead::TransitionCondition>
get_transition_condition(
    const std::shared_ptr<const sema::ast::action::ReadOctetsUntilTerminator>
        &read_action) {
  return operation::TransitionLookahead::MatchUntilTerminator{
      read_action->terminator};
}

static std::optional<operation::TransitionLookahead::TransitionCondition>
get_transition_condition(
    const std::shared_ptr<const sema::ast::action::ReadStaticOctets>
        &read_action) {
  return read_action->octets;
}

static std::optional<operation::TransitionLookahead::TransitionCondition>
get_transition_condition(const sema::ast::Action &action);

static std::optional<operation::TransitionLookahead::TransitionCondition>
get_transition_condition(
    const std::shared_ptr<const sema::ast::action::Loop> &loop_action) {
  if (!loop_action->actions.empty()) {
    return get_transition_condition(loop_action->actions.front());
  }
  return std::nullopt;
}

static std::optional<operation::TransitionLookahead::TransitionCondition>
get_transition_condition(const auto &) {
  return std::nullopt;
}

static std::optional<operation::TransitionLookahead::TransitionCondition>
get_transition_condition(const sema::ast::Action &action) {
  return std::visit([](const auto &a) { return get_transition_condition(a); },
                    action);
}

static std::optional<operation::TransitionLookahead::TransitionCondition>
get_transition_condition(
    const std::shared_ptr<const sema::ast::WriteTransition> &write_transition) {
  return std::nullopt;
}

static std::optional<operation::TransitionLookahead::TransitionCondition>
get_transition_condition(
    const std::shared_ptr<const sema::ast::ReadTransition> &read_transition) {
  if (!read_transition->actions.empty()) {
    return get_transition_condition(read_transition->actions.front());
  }
  return std::nullopt;
}

static std::optional<std::shared_ptr<const OpTree>>
generate_read_state_callback(
    const std::shared_ptr<const sema::ast::State> &state) {
  std::vector<std::pair<operation::TransitionLookahead::TransitionCondition,
                        std::string>>
      conditions;
  for (const auto &transition_pair : state->transitions) {
    const auto &transition = transition_pair.second.first;
    const auto &target_state = transition_pair.second.second;
    if (target_state == "Closed") {
      conditions.emplace_back(operation::TransitionLookahead::EOFCondition{},
                              transition_pair.first);
    } else {
      auto maybe_condition = std::visit(
          [](auto &t) { return get_transition_condition(t); }, transition);
      if (maybe_condition) {
        conditions.emplace_back(*maybe_condition, transition_pair.first);
      } else {
        return std::nullopt;
      }
    }
  }
  if (conditions.empty()) {
    return std::nullopt;
  }
  return std::make_shared<OpTree>(
      OpTreeNode{operation::DynamicList{},
                 {{operation::TransitionLookahead{conditions}, {}},
                  {operation::DictionaryInitialize{}, {}}}});
}

static std::optional<StateMachineOperation::StateMap> construct_state_map(
    const std::unordered_map<std::string,
                             std::shared_ptr<const sema::ast::State>> &states) {
  StateMachineOperation::StateMap state_map;
  for (const auto &state_pair : states) {
    StateMachineOperation::StateInfo state_info;
    std::optional<TransitionType> state_transitions_type = std::nullopt;
    for (const auto &transition_pair : state_pair.second->transitions) {
      const auto &transition = transition_pair.second.first;
      const auto &target_state = transition_pair.second.second;
      auto maybe_transition =
          std::visit([&](auto &t) { return visit_transition(t, target_state); },
                     transition);
      if (!maybe_transition)
        return std::nullopt;
      auto [transition_type, transition_info] = *maybe_transition;
      state_info.transitions[transition_pair.first] = transition_info;
      if (state_transitions_type.has_value()) {
        if (state_transitions_type != transition_type) {
          return std::nullopt;
        }
      } else {
        state_transitions_type = transition_type;
      }
    }
    if (state_pair.first != "Closed") {
      if (!state_transitions_type)
        state_transitions_type = TransitionType::Read;
      switch (state_transitions_type.value()) {
      case TransitionType::Read: {
        auto maybe_optree = generate_read_state_callback(state_pair.second);
        if (!maybe_optree)
          return std::nullopt;
        state_info.callback_optree = *maybe_optree;
      } break;
      case TransitionType::Write:
        state_info.callback_optree = std::make_shared<OpTree>(
            OpTree(OpTreeNode{UnaryCallback(state_pair.first),
                              {{LexicalPadGet{"dictionary"}, {}}}}));
        break;
      }
    } else {
      state_info.callback_optree = std::make_shared<OpTree>(
          OpTree(OpTreeNode{UnaryCallback(state_pair.first),
                            {{LexicalPadGet{"dictionary"}, {}}}}));
    }
    state_map[state_pair.first] = state_info;
  }
  return state_map.empty()
             ? std::nullopt
             : std::optional<StateMachineOperation::StateMap>{state_map};
}

std::optional<std::shared_ptr<const OpTree>>
client(const std::shared_ptr<const sema::ast::Protocol> &protocol) {
  if (!protocol || !protocol->client) {
    return std::nullopt;
  }

  auto state_map_opt = construct_state_map(protocol->client->states);
  if (!state_map_opt) {
    return std::nullopt;
  }

  auto state_machine_operation = StateMachineOperation(*state_map_opt);
  auto optree =
      std::make_shared<OpTree>(OpTree{OpTreeNode{state_machine_operation, {}}});
  return optree;
}

std::optional<std::shared_ptr<const OpTree>>
server(const std::shared_ptr<const sema::ast::Protocol> &protocol) {
  if (!protocol || !protocol->server) {
    return std::nullopt;
  }

  auto state_map_opt = construct_state_map(protocol->server->states);
  if (!state_map_opt) {
    return std::nullopt;
  }

  auto state_machine_operation = StateMachineOperation(*state_map_opt);
  auto optree =
      std::make_shared<OpTree>(OpTree{OpTreeNode{state_machine_operation, {}}});
  return optree;
}

} // namespace generate
} // namespace networkprotocoldsl
