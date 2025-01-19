#include <networkprotocoldsl/operation/functioncall.hpp>
#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cassert>
#include <type_traits>
#include <variant>

namespace networkprotocoldsl::operation {

static OperationResult _function_call(ControlFlowOperationContext &ctx,
                                      value::Callable callable,
                                      value::DynamicList arglist) {
  ctx.callable = callable;
  ctx.arglist = arglist.values;
  return ReasonForBlockedOperation::WaitingForCallableInvocation;
}

static OperationResult _function_call(ControlFlowOperationContext &ctx,
                                      value::Callable a,
                                      value::RuntimeError b) {
  return b;
}

static OperationResult _function_call(ControlFlowOperationContext &ctx, auto &,
                                      auto &) {
  return value::RuntimeError::TypeError;
}

OperationResult FunctionCall::operator()(ControlFlowOperationContext &ctx,
                                         Arguments a) const {
  if (ctx.callable.has_value()) {
    if (ctx.callable_invoked) {
      if (ctx.value.has_value()) {
        return ctx.value.value();
      } else {
        return ReasonForBlockedOperation::WaitingForCallableResult;
      }
    } else {
      return ReasonForBlockedOperation::WaitingForCallableInvocation;
    }
  } else {
    return std::visit(
        [&ctx](auto &callable, auto &arglist) {
          return _function_call(ctx, callable, arglist);
        },
        std::get<0>(a), std::get<1>(a));
  }
}

Value FunctionCall::get_callable(ControlFlowOperationContext &ctx) const {
  assert(ctx.callable.has_value());
  return ctx.callable.value();
}
void FunctionCall::set_callable_invoked(
    ControlFlowOperationContext &ctx) const {
  ctx.callable_invoked = true;
}
void FunctionCall::set_callable_return(ControlFlowOperationContext &ctx,
                                       Value v) const {
  ctx.value = v;
}

std::shared_ptr<const std::vector<Value>>
FunctionCall::get_argument_list(ControlFlowOperationContext &ctx) const {
  return ctx.arglist;
}

} // namespace networkprotocoldsl::operation
