#include <networkprotocoldsl/operation/statemachineoperation.hpp>

#include <networkprotocoldsl/operation/staticcallable.hpp>
#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/print_optreenode.hpp>
#include <networkprotocoldsl/value.hpp>

#include <ostream>
#include <thread>

#include <cassert>

//#define DEBUG(x)
#define DEBUG(x)                                                               \
  std::cerr << std::this_thread::get_id() << ": " << x << std::endl;

namespace {

using namespace networkprotocoldsl;
using namespace networkprotocoldsl::operation;

enum class ProcessingMode {
  Transition,
  State,
};

using StatePair = std::pair<std::string, StateMachineOperation::StateInfo>;
using TransitionPair =
    std::pair<std::string, StateMachineOperation::TransitionInfo>;

struct ProcessingInfo {
  ProcessingMode processing_mode;
  std::optional<StatePair> from_state;
  std::optional<TransitionPair> transition;
  StatePair to_state;
};

} // namespace

namespace networkprotocoldsl::operation {

static std::optional<std::shared_ptr<std::vector<Value>>>
_extract_arguments(const std::vector<std::string> &names,
                   const value::Dictionary &d) {
  DEBUG("Entering _extract_arguments");
  auto args = std::make_shared<std::vector<Value>>();
  for (const auto &arg_name : names) {
    auto it = d.members->find(arg_name);
    if (it == d.members->end()) {
      args->push_back(false);
    } else {
      args->push_back(it->second);
    }
  }
  return args;
}

static OperationResult
_start_state_callback(ControlFlowOperationContext &ctx,
                      const StateMachineOperation::StateMap &states) {
  DEBUG("Entering _start_state_callback");
  ProcessingInfo &info = std::any_cast<ProcessingInfo &>(ctx.additional_info);
  auto &s = info.to_state.second;
  ctx.callable = value::Callable{s.callback_optree, {"dictionary"}, true};
  ctx.value = std::nullopt;
  info.processing_mode = ProcessingMode::State;
  return ReasonForBlockedOperation::WaitingForCallableInvocation;
}

static OperationResult
_start_transition_callback(ControlFlowOperationContext &ctx,
                           const StateMachineOperation::StateMap &states) {
  DEBUG("Entering _start_transition_callback");
  ProcessingInfo &info = std::any_cast<ProcessingInfo &>(ctx.additional_info);
  assert(info.transition.has_value());
  auto &t = info.transition.value().second;
  ctx.callable = value::Callable{t.callback_optree, t.argument_names, true};
  ctx.value = std::nullopt;
  info.processing_mode = ProcessingMode::Transition;
  return ReasonForBlockedOperation::WaitingForCallableInvocation;
}

static OperationResult
_process_state_return_with_args(ControlFlowOperationContext &ctx,
                                const StateMachineOperation::StateMap &states,
                                const value::Octets t,
                                const value::Dictionary d) {
  DEBUG("Entering _process_state_return_with_args (Octets, Dictionary)");
  ProcessingInfo &info = std::any_cast<ProcessingInfo &>(ctx.additional_info);
  if (info.to_state.first == "Closed") {
    return d;
  }
  auto &s = info.to_state.second;
  info.from_state = info.to_state;
  auto it = s.transitions.find(*t.data);
  if (it == s.transitions.end()) {
    return value::RuntimeError::NameError;
  }
  auto target_state_name = it->second.target_state;
  auto it2 = states.find(target_state_name);
  if (it2 == states.end()) {
    return value::RuntimeError::NameError;
  }
  auto maybe_args = _extract_arguments(it->second.argument_names, d);
  if (!maybe_args.has_value()) {
    return value::RuntimeError::TypeError;
  }
  ctx.accumulator = maybe_args.value();
  info.transition = *it;
  info.to_state = *it2;
  return _start_transition_callback(ctx, states);
}

static OperationResult
_process_state_return_with_args(ControlFlowOperationContext &ctx,
                                const StateMachineOperation::StateMap &states,
                                const auto &, const auto &) {
  DEBUG("Entering _process_state_return_with_args (auto, auto)");
  return value::RuntimeError::TypeError;
}

static OperationResult
_process_state_return_dynlist(ControlFlowOperationContext &ctx,
                              const StateMachineOperation::StateMap &states,
                              const value::DynamicList v) {
  DEBUG("Entering _process_state_return_dynlist (DynamicList)");
  if (v.values->size() != 2) {
    return value::RuntimeError::TypeError;
  }
  return std::visit(
      [&](auto &t, auto &d) {
        return _process_state_return_with_args(ctx, states, t, d);
      },
      v.values->at(0), v.values->at(1));
}

static OperationResult
_process_state_return_dynlist(ControlFlowOperationContext &ctx,
                              const StateMachineOperation::StateMap &states,
                              const value::RuntimeError err) {
  DEBUG("Entering _process_state_return_dynlist (RuntimeError)");
  return err;
}

static OperationResult
_process_state_return_dynlist(ControlFlowOperationContext &ctx,
                              const StateMachineOperation::StateMap &states,
                              const auto &) {
  DEBUG("Entering _process_state_return_dynlist (auto)");
  return value::RuntimeError::TypeError;
}

static OperationResult
_process_state_return(ControlFlowOperationContext &ctx,
                      const StateMachineOperation::StateMap &states) {
  DEBUG("Entering _process_state_return");
  assert(ctx.value.has_value());
  return std::visit(
      [&](auto &v) { return _process_state_return_dynlist(ctx, states, v); },
      ctx.value.value());
}

static OperationResult _process_transition_return_with_args(
    ControlFlowOperationContext &ctx,
    const StateMachineOperation::StateMap &states, const value::Dictionary &d) {
  DEBUG("Entering _process_transition_return_with_args (Dictionary)");
  ctx.accumulator = std::make_shared<std::vector<Value>>(std::vector<Value>{d});
  return _start_state_callback(ctx, states);
}

static OperationResult _process_transition_return_with_args(
    ControlFlowOperationContext &ctx,
    const StateMachineOperation::StateMap &states,
    const value::RuntimeError &err) {
  DEBUG("Entering _process_transition_return_with_args (RuntimeError)");
  return err;
}

static OperationResult _process_transition_return_with_args(
    ControlFlowOperationContext &ctx,
    const StateMachineOperation::StateMap &states, const auto &) {
  DEBUG("Entering _process_transition_return_with_args (auto)");
  return value::RuntimeError::TypeError;
}

static OperationResult _process_transition_return_dynlist(
    ControlFlowOperationContext &ctx,
    const StateMachineOperation::StateMap &states,
    const value::DynamicList &l) {
  DEBUG("Entering _process_transition_return_dynlist (DynamicList)");
  if (l.values->size() != 1) {
    return value::RuntimeError::TypeError;
  }
  return std::visit(
      [&](auto &v) {
        return _process_transition_return_with_args(ctx, states, v);
      },
      l.values->at(0));
}

static OperationResult _process_transition_return_dynlist(
    ControlFlowOperationContext &ctx,
    const StateMachineOperation::StateMap &states,
    const value::RuntimeError err) {
  DEBUG("Entering _process_transition_return_dynlist (RuntimeError)");
  return err;
}

static OperationResult _process_transition_return_dynlist(
    ControlFlowOperationContext &ctx,
    const StateMachineOperation::StateMap &states, const auto &) {
  DEBUG("Entering _process_transition_return_dynlist (auto)");
  return value::RuntimeError::TypeError;
}

static OperationResult
_process_transition_return(ControlFlowOperationContext &ctx,
                           const StateMachineOperation::StateMap &states) {
  DEBUG("Entering _process_transition_return");
  assert(ctx.value.has_value());
  return std::visit(
      [&](auto &v) {
        return _process_transition_return_dynlist(ctx, states, v);
      },
      ctx.value.value());
}

OperationResult
StateMachineOperation::operator()(ControlFlowOperationContext &ctx,
                                  Arguments a) const {
  DEBUG("Entering StateMachineOperation::operator()");
  if (ctx.callable.has_value()) {
    if (ctx.callable_invoked) {
      if (ctx.value.has_value()) {
        ProcessingInfo &info =
            std::any_cast<ProcessingInfo &>(ctx.additional_info);
        if (info.processing_mode == ProcessingMode::Transition) {
          if (!info.transition.has_value()) {
            return value::RuntimeError::NameError;
          }
          return _process_transition_return(ctx, states);
        } else {
          return _process_state_return(ctx, states);
        }
      } else {
        return ReasonForBlockedOperation::WaitingForCallableResult;
      }
    } else {
      return ReasonForBlockedOperation::WaitingForCallableInvocation;
    }
  } else {
    auto it = states.find("Open");
    if (it == states.end()) {
      return value::RuntimeError::NameError;
    }
    ctx.accumulator = std::make_shared<std::vector<Value>>(
        std::vector<Value>{value::Dictionary()});
    ctx.additional_info = ProcessingInfo{ProcessingMode::Transition,
                                         std::nullopt, std::nullopt, *it};
    return _start_state_callback(ctx, states);
  }
}

Value StateMachineOperation::get_callable(
    ControlFlowOperationContext &ctx) const {
  DEBUG("Entering StateMachineOperation::get_callable");
  assert(ctx.callable.has_value());
  return ctx.callable.value();
}

std::shared_ptr<const std::vector<Value>>
StateMachineOperation::get_argument_list(
    ControlFlowOperationContext &ctx) const {
  DEBUG("Entering StateMachineOperation::get_argument_list");
  return ctx.accumulator;
}

void StateMachineOperation::set_callable_invoked(
    ControlFlowOperationContext &ctx) const {
  DEBUG("Entering StateMachineOperation::set_callable_invoked");
  ctx.callable_invoked = true;
}

void StateMachineOperation::set_callable_return(
    ControlFlowOperationContext &ctx, Value v) const {
  DEBUG("Entering StateMachineOperation::set_callable_return");
  ctx.value = v;
}

std::string StateMachineOperation::stringify() const {
  DEBUG("Entering StateMachineOperation::stringify");
  std::ostringstream os;
  os << "StateMachineOperation{\n";
  for (const auto &[state_name, state_info] : states) {
    os << "  State: " << state_name << "\n";
    os << "    Callback Optree:\n";
    if (state_info.callback_optree) {
      print_optreenode(state_info.callback_optree->root, os, "     │ ",
                       "     │ ");
    } else {
      os << "      <None>\n";
    }
    os << "    Transitions:\n";
    for (const auto &[transition_name, transition_info] :
         state_info.transitions) {
      os << "      Transition: " << transition_name << " -> "
         << transition_info.target_state << "\n";
      os << "        Argument Names: [";
      for (size_t i = 0; i < transition_info.argument_names.size(); ++i) {
        os << "\"" << transition_info.argument_names[i] << "\"";
        if (i < transition_info.argument_names.size() - 1) {
          os << ", ";
        }
      }
      os << "]\n";
      os << "        Callback Optree:\n";
      if (transition_info.callback_optree) {
        print_optreenode(transition_info.callback_optree->root, os,
                         "         │ ", "         │ ");
      } else {
        os << "          <None>\n";
      }
    }
  }
  os << "}";
  return os.str();
}

} // namespace networkprotocoldsl::operation