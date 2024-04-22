#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>
#include <memory>
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

template <ControlFlowOperationConcept O>
static OperationContextVariant initialize_context(const O &o) {
  return ControlFlowOperationContext();
}

template <CallbackOperationConcept O>
static OperationContextVariant initialize_context(const O &o) {
  return CallbackOperationContext();
}

template <InputOutputOperationConcept O>
static OperationContextVariant initialize_context(const O &o) {
  return InputOutputOperationContext();
}

template <LexicalPadOperationConcept O>
static OperationContextVariant initialize_context(const O &o) {
  return false;
}

template <DynamicInputOperationConcept O>
static OperationContextVariant initialize_context(const O &o) {
  return false;
}

template <OperationConcept O>
static bool operation_has_arguments_ready(ExecutionStackFrame *frame,
                                          const O &o) {
  if (frame->get_accumulator()->size() <
      std::tuple_size<typename O::Arguments>::value) {
    return false;
  } else {
    return true;
  }
}

template <DynamicInputOperationConcept O>
static bool operation_has_arguments_ready(ExecutionStackFrame *frame,
                                          const O &o) {
  std::shared_ptr<std::vector<Value>> acc = frame->get_accumulator();
  if (acc->size() < frame->get_children_count() &&
      !(acc->size() > 0 &&
        (std::holds_alternative<value::RuntimeError>(acc->back()) ||
         std::holds_alternative<value::ControlFlowInstruction>(acc->back())))) {
    assert(acc->size() < frame->get_children_count());
    return false;
  } else {
    return true;
  }
}

template <InterpretedOperationConcept O>
static OperationResult execute_specific_operation(ExecutionStackFrame *frame,
                                                  const O &o) {
  typename O::Arguments args(make_argument_tuple(
      *(frame->get_accumulator()),
      std::make_index_sequence<
          std::tuple_size<typename O::Arguments>::value>()));
  return o(args);
}

template <CallbackOperationConcept O>
static OperationResult execute_specific_operation(ExecutionStackFrame *frame,
                                                  const O &o) {
  typename O::Arguments args(make_argument_tuple(
      *(frame->get_accumulator()),
      std::make_index_sequence<
          std::tuple_size<typename O::Arguments>::value>()));
  return o(std::get<CallbackOperationContext>(frame->get_context()), args);
}

template <ControlFlowOperationConcept O>
static OperationResult execute_specific_operation(ExecutionStackFrame *frame,
                                                  const O &o) {
  typename O::Arguments args(make_argument_tuple(
      *(frame->get_accumulator()),
      std::make_index_sequence<
          std::tuple_size<typename O::Arguments>::value>()));
  return o(std::get<ControlFlowOperationContext>(frame->get_context()), args);
}

template <InputOutputOperationConcept O>
static OperationResult execute_specific_operation(ExecutionStackFrame *frame,
                                                  const O &o) {
  typename O::Arguments args(make_argument_tuple(
      *(frame->get_accumulator()),
      std::make_index_sequence<
          std::tuple_size<typename O::Arguments>::value>()));
  return o(std::get<InputOutputOperationContext>(frame->get_context()), args);
}

template <LexicalPadOperationConcept O>
static OperationResult execute_specific_operation(ExecutionStackFrame *frame,
                                                  const O &o) {
  typename O::Arguments args(make_argument_tuple(
      *(frame->get_accumulator()),
      std::make_index_sequence<
          std::tuple_size<typename O::Arguments>::value>()));
  return o(args, frame->get_pad());
}

template <DynamicInputOperationConcept O>
static OperationResult execute_specific_operation(ExecutionStackFrame *frame,
                                                  const O &o) {
  return o(frame->get_accumulator());
}

ExecutionStackFrame::ExecutionStackFrame(const OpTreeNode &o,
                                         std::shared_ptr<LexicalPad> p)
    : optreenode(o), pad(p) {
  ctx = std::visit([](auto &o) { return initialize_context(o); },
                   optreenode.operation);
  accumulator = std::make_shared<std::vector<Value>>();
}

bool ExecutionStackFrame::has_arguments_ready() {
  return std::visit(
      [this](auto &o) { return operation_has_arguments_ready(this, o); },
      optreenode.operation);
}

std::shared_ptr<LexicalPad> ExecutionStackFrame::get_pad() { return pad; }

OperationResult ExecutionStackFrame::execute() {
  assert(has_arguments_ready());
  return std::visit(
      [this](auto &o) { return execute_specific_operation(this, o); },
      optreenode.operation);
}

void ExecutionStackFrame::push_back(Value v) { accumulator->push_back(v); }

const OpTreeNode &ExecutionStackFrame::next_op() {
  assert(!has_arguments_ready());
  assert(accumulator->size() < optreenode.children.size());
  return optreenode.children[accumulator->size()];
}

const Operation &ExecutionStackFrame::get_operation() {
  return optreenode.operation;
}

size_t ExecutionStackFrame::get_children_count() {
  return optreenode.children.size();
}

OperationContextVariant &ExecutionStackFrame::get_context() { return ctx; }

std::shared_ptr<std::vector<Value>> ExecutionStackFrame::get_accumulator() {
  return accumulator;
}

} // namespace networkprotocoldsl
