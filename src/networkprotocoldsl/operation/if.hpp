#ifndef NETWORKPROTOCOLDSL_OPERATION_IF_HPP
#define NETWORKPROTOCOLDSL_OPERATION_IF_HPP

#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <variant>

namespace networkprotocoldsl {

namespace operation {

/**
 * Compares two values.
 */
class If {
public:
  using Arguments = std::tuple<Value, Value, Value>;
  OperationResult operator()(ControlFlowOperationContext &ctx,
                             Arguments a) const;
  Value get_callable(ControlFlowOperationContext &ctx) const;
  std::shared_ptr<const std::vector<Value>>
  get_argument_list(ControlFlowOperationContext &ctx) const;
  void set_callable_invoked(ControlFlowOperationContext &ctx) const;
  void set_callable_return(ControlFlowOperationContext &ctx, Value v) const;
  std::string stringify() const { return "If{}"; }
};
static_assert(ControlFlowOperationConcept<If>);

}; // namespace operation

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_OPERATION_HPP
