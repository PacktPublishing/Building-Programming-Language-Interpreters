#ifndef INCLUDED_NETWORKPROTOCOLDSL_EXECUTIONSTACKFRAME_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_EXECUTIONSTACKFRAME_HPP

#include <networkprotocoldsl/lexicalpad.hpp>
#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/optree.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <tuple>

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
  std::shared_ptr<std::vector<Value>> accumulator;
  OperationContextVariant ctx;
  std::shared_ptr<LexicalPad> pad;

public:
  ExecutionStackFrame(const OpTreeNode &o, std::shared_ptr<LexicalPad> pad);

  bool has_arguments_ready();

  OperationResult execute();

  void push_back(Value v);

  const OpTreeNode &next_op();

  std::shared_ptr<LexicalPad> get_pad();

  const Operation &get_operation();

  size_t get_children_count();

  OperationContextVariant &get_context();

  std::shared_ptr<std::vector<Value>> get_accumulator();
};

} // namespace networkprotocoldsl

#endif
