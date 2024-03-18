#ifndef INCLUDED_NETWORKPROTOCOLDSL_OPERATION_FUNCTIONCALL_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_OPERATION_FUNCTIONCALL_HPP

#include <networkprotocoldsl/operationconcepts.hpp>

namespace networkprotocoldsl::operation {

class FunctionCall {
public:
  using Arguments = std::tuple<Value, Value>;
  OperationResult operator()(ControlFlowOperationContext &ctx,
                             Arguments a) const;
  Value get_callable(ControlFlowOperationContext &ctx) const;
  std::shared_ptr<std::vector<Value>>
  get_argument_list(ControlFlowOperationContext &ctx) const;
  void set_callable_invoked(ControlFlowOperationContext &ctx) const;
  void set_callable_return(ControlFlowOperationContext &ctx, Value v) const;
};
static_assert(ControlFlowOperationConcept<FunctionCall>);

} // namespace networkprotocoldsl::operation

#endif