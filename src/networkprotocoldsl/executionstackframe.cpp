#include <networkprotocoldsl/executionstackframe.hpp>

#include <cassert>

namespace networkprotocoldsl {

template <std::size_t... Indices>
static auto make_argument_tuple(const std::vector<Value> &v,
                                std::index_sequence<Indices...>) {
  return std::make_tuple(v[Indices]...);
}

template <InterpretedOperationConcept O>
static OperationContextVariant initialize_context(const O &o) {
  return false;
}

template <CallbackOperationConcept O>
static OperationContextVariant initialize_context(const O &o) {
  return CallbackOperationContext();
}

template <InputOutputOperationConcept O>
static OperationContextVariant initialize_context(const O &o) {
  return InputOutputOperationContext();
}

template <OperationConcept O>
static bool operation_has_arguments_ready(ExecutionStackFrame *frame,
                                          const O &o) {
  if (frame->get_accumulator().size() <
      std::tuple_size<typename O::Arguments>::value) {
    return false;
  } else {
    return true;
  }
}

template <InterpretedOperationConcept O>
static OperationResult execute_specific_operation(ExecutionStackFrame *frame,
                                                  const O &o) {
  typename O::Arguments args(make_argument_tuple(
      frame->get_accumulator(),
      std::make_index_sequence<
          std::tuple_size<typename O::Arguments>::value>()));
  return o(args);
}

template <CallbackOperationConcept O>
static OperationResult execute_specific_operation(ExecutionStackFrame *frame,
                                                  const O &o) {
  typename O::Arguments args(make_argument_tuple(
      frame->get_accumulator(),
      std::make_index_sequence<
          std::tuple_size<typename O::Arguments>::value>()));
  return o(std::get<CallbackOperationContext>(frame->get_context()), args);
}

template <InputOutputOperationConcept O>
static OperationResult execute_specific_operation(ExecutionStackFrame *frame,
                                                  const O &o) {
  typename O::Arguments args(make_argument_tuple(
      frame->get_accumulator(),
      std::make_index_sequence<
          std::tuple_size<typename O::Arguments>::value>()));
  return o(std::get<InputOutputOperationContext>(frame->get_context()), args);
}

ExecutionStackFrame::ExecutionStackFrame(const OpTreeNode &o) : optreenode(o) {
  ctx = std::visit([this](auto &o) { return initialize_context(o); },
                   optreenode.operation);
}

bool ExecutionStackFrame::has_arguments_ready() {
  return std::visit(
      [this](auto &o) { return operation_has_arguments_ready(this, o); },
      optreenode.operation);
}

OperationResult ExecutionStackFrame::execute() {
  assert(has_arguments_ready());
  return std::visit(
      [this](auto &o) { return execute_specific_operation(this, o); },
      optreenode.operation);
}

void ExecutionStackFrame::push_back(Value v) { accumulator.push_back(v); }

const OpTreeNode &ExecutionStackFrame::next_op() {
  assert(!has_arguments_ready());
  return optreenode.children[accumulator.size()];
}

const Operation &ExecutionStackFrame::get_operation() {
  return optreenode.operation;
}

OperationContextVariant &ExecutionStackFrame::get_context() { return ctx; }

std::vector<Value> &ExecutionStackFrame::get_accumulator() {
  return accumulator;
}

} // namespace networkprotocoldsl
