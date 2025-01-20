#include <networkprotocoldsl/operation/functioncallforeach.hpp>
#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cassert>
#include <memory>
#include <type_traits>
#include <variant>

namespace networkprotocoldsl::operation {

static OperationResult _function_call_for_each(ControlFlowOperationContext &ctx,
                                               value::Callable callable,
                                               value::DynamicList arglist) {
  ctx.callable = callable;
  ctx.arglist = arglist.values;
  ctx.accumulator = std::make_shared<std::vector<Value>>();
  return ReasonForBlockedOperation::WaitingForCallableInvocation;
}

static OperationResult _function_call_for_each(ControlFlowOperationContext &ctx,
                                               value::Callable callable,
                                               Value arglist) {
  return std::visit(
      [&ctx, &callable](auto arglist_visited) {
        return _function_call_for_each(ctx, callable, arglist_visited);
      },
      arglist);
}

static OperationResult _function_call_for_each(ControlFlowOperationContext &ctx,
                                               auto a, auto b) {
  return value::RuntimeError::TypeError;
}

OperationResult
FunctionCallForEach::operator()(ControlFlowOperationContext &ctx,
                                Arguments a) const {
  if (ctx.callable.has_value()) {
    if (ctx.callable_invoked) {
      if (ctx.value.has_value()) {
        ctx.accumulator->push_back(ctx.value.value());
        if (ctx.accumulator->size() < ctx.arglist->size()) {
          ctx.value.reset();
          ctx.callable_invoked = false;
          return ReasonForBlockedOperation::WaitingForCallableInvocation;
        } else {
          return value::DynamicList{ctx.accumulator};
        }
      } else {
        return ReasonForBlockedOperation::WaitingForCallableResult;
      }
    } else {
      return ReasonForBlockedOperation::WaitingForCallableInvocation;
    }
  } else {
    return std::visit(
        [&ctx, &a](auto &callable) {
          return _function_call_for_each(ctx, callable, std::get<1>(a));
        },
        std::get<0>(a));
  }
}

Value FunctionCallForEach::get_callable(
    ControlFlowOperationContext &ctx) const {
  assert(ctx.callable.has_value());
  return ctx.callable.value();
}
void FunctionCallForEach::set_callable_invoked(
    ControlFlowOperationContext &ctx) const {
  ctx.callable_invoked = true;
}
void FunctionCallForEach::set_callable_return(ControlFlowOperationContext &ctx,
                                              Value v) const {
  ctx.value = v;
}

std::shared_ptr<const std::vector<Value>>
FunctionCallForEach::get_argument_list(ControlFlowOperationContext &ctx) const {
  if (element_is_single_argument) {
    return std::make_shared<std::vector<Value>>(
        std::vector<Value>{ctx.arglist->at(ctx.accumulator->size())});
  } else {
    // This presumes that this is a list of lists. A different operation would
    // be needed in order to support cases where that's not going to be true.
    return std::get<value::DynamicList>(
               ctx.arglist->at(ctx.accumulator->size()))
        .values;
  }
}

} // namespace networkprotocoldsl::operation
