#ifndef INCLUDED_NETWORKPROTOCOLDSL_EXECUTIONSTACKFRAME_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_EXECUTIONSTACKFRAME_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <tuple>

#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/optree.hpp>

namespace networkprotocoldsl {

using OperationContextVariant =
    std::variant<bool, CallbackOperationContext, ControlFlowOperationContext,
                 InputOutputOperationContext>;

/**
 * The execution frame points to a specific operation and the
 * accumulation of inputs for that operation. The actual operation can
 * only happen when the right number of inputs has been provided.
 */
class ExecutionStackFrame {
  const OpTreeNode &optreenode;
  std::vector<Value> accumulator;
  OperationContextVariant ctx;

public:
  ExecutionStackFrame(const OpTreeNode &o);

  bool has_arguments_ready();

  OperationResult execute();

  void push_back(Value v);

  const OpTreeNode &next_op();

  const Operation &get_operation();

  OperationContextVariant &get_context();

  std::vector<Value> &get_accumulator();
};

} // namespace networkprotocoldsl

#endif
