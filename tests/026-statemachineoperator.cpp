#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/operation/statemachineoperation.hpp>
#include <networkprotocoldsl/value.hpp>

#include <gtest/gtest.h>
#include <variant>

using namespace networkprotocoldsl;
using namespace networkprotocoldsl::operation;

TEST(StateMachineOperationTest, BasicStateTransition) {
  // Define the state machine
  StateMachineOperation::StateMap states;

  // Define the "Open" state
  StateMachineOperation::StateInfo openState;
  openState.callback_optree = nullptr;
  openState.transitions["close"] =
      StateMachineOperation::TransitionInfo{nullptr, {}, "Closed"};

  // Define the "Closed" state
  StateMachineOperation::StateInfo closedState;
  closedState.callback_optree = nullptr;
  closedState.transitions = {};

  // Add states to the state map
  states["Open"] = openState;
  states["Closed"] = closedState;

  // Create the state machine operation
  StateMachineOperation stateMachine(states);

  // Create a mock context
  ControlFlowOperationContext ctx;

  // Execute the state machine operation
  auto result = stateMachine(ctx, {});

  // Verify the initial state is "Open"
  ASSERT_TRUE(std::holds_alternative<ReasonForBlockedOperation>(result));
  ASSERT_EQ(std::get<ReasonForBlockedOperation>(result),
            ReasonForBlockedOperation::WaitingForCallableInvocation);
  ASSERT_EQ(ctx.callable.has_value(), true);
  ASSERT_EQ(ctx.value.has_value(), false);

  // Simulate invoking the callable
  stateMachine.set_callable_invoked(ctx);

  // Simulate returning a value from the callable
  stateMachine.set_callable_return(
      ctx, value::DynamicList{
               std::make_shared<std::vector<Value>>(std::vector<Value>{
                   value::Octets{std::make_shared<std::string>("close")},
                   value::Dictionary{}})});

  // Execute the state machine operation again
  result = stateMachine(ctx, {});

  // Verify the callback to the transition
  ASSERT_TRUE(std::holds_alternative<ReasonForBlockedOperation>(result));
  ASSERT_EQ(std::get<ReasonForBlockedOperation>(result),
            ReasonForBlockedOperation::WaitingForCallableInvocation);
  ASSERT_EQ(ctx.callable.has_value(), true);
  ASSERT_EQ(ctx.value.has_value(), false);

  // Simulate invoking the callable for the transition
  stateMachine.set_callable_invoked(ctx);

  // Simulate returning a value from the callable
  stateMachine.set_callable_return(
      ctx, value::DynamicList{std::make_shared<std::vector<Value>>(
               std::vector<Value>{value::Dictionary{}})});

  // Execute the state machine operation again
  result = stateMachine(ctx, {});

  // Verify the state transition to "Closed"
  ASSERT_TRUE(std::holds_alternative<ReasonForBlockedOperation>(result));
  ASSERT_EQ(std::get<ReasonForBlockedOperation>(result),
            ReasonForBlockedOperation::WaitingForCallableInvocation);
  ASSERT_EQ(ctx.callable.has_value(), true);
  ASSERT_EQ(ctx.value.has_value(), false);

  // Simulate invoking the callable in the "Closed" state
  stateMachine.set_callable_invoked(ctx);

  // Simulate returning a value from the callable
  stateMachine.set_callable_return(
      ctx,
      value::DynamicList{std::make_shared<std::vector<Value>>(
          std::vector<Value>{value::Octets{std::make_shared<std::string>("")},
                             value::Dictionary{}})});

  // Execute the state machine operation again
  result = stateMachine(ctx, {});

  // Verify the final state is "Closed"
  ASSERT_TRUE(std::holds_alternative<Value>(result));
  auto v = std::get<Value>(result);
  ASSERT_TRUE(std::holds_alternative<value::Dictionary>(v));
}