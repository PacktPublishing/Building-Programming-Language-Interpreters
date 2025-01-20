#include <networkprotocoldsl/operation/generatelist.hpp>
#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cassert>
#include <memory>
#include <type_traits>
#include <variant>

namespace networkprotocoldsl::operation {

static OperationResult _generate_list(ControlFlowOperationContext &ctx,
                                      value::Callable callable) {
  ctx.callable = callable;
  ctx.accumulator = std::make_shared<std::vector<Value>>(std::vector<Value>());
  return ReasonForBlockedOperation::WaitingForCallableInvocation;
}

static OperationResult _generate_list(ControlFlowOperationContext &ctx,
                                      auto a) {
  return value::RuntimeError::TypeError;
}

OperationResult GenerateList::operator()(ControlFlowOperationContext &ctx,
                                         Arguments a) const {
  if (ctx.callable.has_value()) {
    if (ctx.callable_invoked) {
      if (ctx.value.has_value()) {
        // at this point we check if this is the control flow instruction, and
        // finish the list. Otherwise, we add it to the accumulator.
        if (std::holds_alternative<value::ControlFlowInstruction>(
                ctx.value.value())) {
          if (std::get<value::ControlFlowInstruction>(ctx.value.value()) ==
              value::ControlFlowInstruction::InterruptGenerator) {
            return value::DynamicList{ctx.accumulator};
          }
        }
        // if it's a runtime error, we should also finish the list.
        if (std::holds_alternative<value::RuntimeError>(ctx.value.value())) {
          return ctx.value.value();
        }
        // if it's not the control flow instruction, we reset the context.
        ctx.accumulator->push_back(ctx.value.value());
        ctx.callable_invoked = false;
        ctx.value.reset();
        return ReasonForBlockedOperation::WaitingForCallableInvocation;
      } else {
        return ReasonForBlockedOperation::WaitingForCallableResult;
      }
    } else {
      return ReasonForBlockedOperation::WaitingForCallableInvocation;
    }
  } else {
    return std::visit(
        [&ctx](auto &callable) { return _generate_list(ctx, callable); },
        std::get<0>(a));
  }
}

Value GenerateList::get_callable(ControlFlowOperationContext &ctx) const {
  assert(ctx.callable.has_value());
  return ctx.callable.value();
}
void GenerateList::set_callable_invoked(
    ControlFlowOperationContext &ctx) const {
  ctx.callable_invoked = true;
}
void GenerateList::set_callable_return(ControlFlowOperationContext &ctx,
                                       Value v) const {
  ctx.value = v;
}

std::shared_ptr<const std::vector<Value>>
GenerateList::get_argument_list(ControlFlowOperationContext &ctx) const {
  return ctx.arglist;
}

} // namespace networkprotocoldsl::operation
