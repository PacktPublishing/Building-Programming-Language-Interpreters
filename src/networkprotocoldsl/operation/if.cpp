#include <memory>
#include <networkprotocoldsl/operation/if.hpp>

#include <cassert>
#include <vector>

namespace networkprotocoldsl::operation {

static OperationResult _if(ControlFlowOperationContext &ctx, bool cond,
                           value::Callable _then, value::Callable _else) {
  if (cond) {
    ctx.callable = _then;
  } else {
    ctx.callable = _else;
  }
  return ReasonForBlockedOperation::WaitingForCallableInvocation;
}

static OperationResult _if(ControlFlowOperationContext &ctx,
                           value::RuntimeError r, auto _then, auto _else) {
  return r;
}

static OperationResult _if(ControlFlowOperationContext &ctx, auto cond,
                           value::RuntimeError r, auto _else) {
  return r;
}

static OperationResult _if(ControlFlowOperationContext &ctx, auto cond,
                           auto _then, value::RuntimeError r) {
  return r;
}

static OperationResult _if(ControlFlowOperationContext &ctx, auto cond,
                           auto _then, auto _else) {
  return value::RuntimeError::TypeError;
}

static OperationResult _if(ControlFlowOperationContext &ctx, bool cond,
                           value::Callable _then, Value _else) {
  return std::visit(
      [&ctx, &cond, &_then](auto _e) { return _if(ctx, cond, _then, _e); },
      _else);
}

static OperationResult _if(ControlFlowOperationContext &ctx, bool cond,
                           Value _then, auto _else) {
  return std::visit(
      [&ctx, &cond, &_else](auto _t) { return _if(ctx, cond, _t, _else); },
      _then);
}

OperationResult If::operator()(ControlFlowOperationContext &ctx,
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
        [&ctx, &a](auto lhs) {
          return _if(ctx, lhs, std::get<1>(a), std::get<2>(a));
        },
        std::get<0>(a));
  }
}

Value If::get_callable(ControlFlowOperationContext &ctx) const {
  assert(ctx.callable.has_value());
  return ctx.callable.value();
}

void If::set_callable_invoked(ControlFlowOperationContext &ctx) const {
  ctx.callable_invoked = true;
}

void If::set_callable_return(ControlFlowOperationContext &ctx, Value v) const {
  ctx.value = v;
}

std::shared_ptr<const std::vector<Value>>
If::get_argument_list(ControlFlowOperationContext &ctx) const {
  return ctx.arglist;
}

} // namespace networkprotocoldsl::operation
