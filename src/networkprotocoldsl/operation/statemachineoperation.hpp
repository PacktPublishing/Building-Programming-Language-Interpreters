#ifndef INCLUDED_NETWORKPROTOCOLDSL_OPERATION_STATEMACHINEOPERATION_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_OPERATION_STATEMACHINEOPERATION_HPP

#include <any>
#include <networkprotocoldsl/operationconcepts.hpp>
#include <sstream> // Added for std::ostringstream
#include <unordered_map>

namespace networkprotocoldsl {
class OpTree; // Forward declaration
}

namespace networkprotocoldsl::operation {

class StateMachineOperation {
public:
  struct TransitionInfo {
    std::shared_ptr<const OpTree> callback_optree;
    std::vector<std::string> argument_names;
    std::string target_state;
  };

  using StateTransitionMap = std::unordered_map<std::string, TransitionInfo>;

  struct StateInfo {
    std::shared_ptr<const OpTree> callback_optree;
    StateTransitionMap transitions;
  };

  using StateMap = std::unordered_map<std::string, StateInfo>;

  using Arguments = std::tuple<>;

  StateMachineOperation(StateMap states) : states(states) {}
  OperationResult operator()(ControlFlowOperationContext &ctx,
                             Arguments a) const;
  Value get_callable(ControlFlowOperationContext &ctx) const;
  std::shared_ptr<const std::vector<Value>>
  get_argument_list(ControlFlowOperationContext &ctx) const;
  void set_callable_invoked(ControlFlowOperationContext &ctx) const;
  void set_callable_return(ControlFlowOperationContext &ctx, Value v) const;
  std::string stringify() const;

private:
  StateMap states;
};
static_assert(ControlFlowOperationConcept<StateMachineOperation>);

} // namespace networkprotocoldsl::operation

#endif