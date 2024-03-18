#include <memory>
#include <networkprotocoldsl/continuation.hpp>
#include <vector>

namespace networkprotocoldsl {

template <typename O> static Value _get_callable(Continuation *c, const O &o) {
  return value::RuntimeError::TypeError;
}

template <ControlFlowOperationConcept O>
static Value _get_callable(Continuation *c, const O &o) {
  return o.get_callable(
      std::get<ControlFlowOperationContext>(c->top().get_context()));
}

template <typename O>
static std::shared_ptr<std::vector<Value>> _get_argument_list(Continuation *c,
                                                              const O &o) {
  return std::make_shared<std::vector<Value>>();
}

template <ControlFlowOperationConcept O>
static std::shared_ptr<std::vector<Value>> _get_argument_list(Continuation *c,
                                                              const O &o) {
  return o.get_argument_list(
      std::get<ControlFlowOperationContext>(c->top().get_context()));
}

template <typename O>
static void _set_callable_invoked(Continuation *c, const O &o) {}

template <ControlFlowOperationConcept O>
static void _set_callable_invoked(Continuation *c, const O &o) {
  return o.set_callable_invoked(
      std::get<ControlFlowOperationContext>(c->top().get_context()));
}

template <typename O>
static void _set_callable_return(Continuation *c, const O &o, Value v) {}

template <ControlFlowOperationConcept O>
static void _set_callable_return(Continuation *c, const O &o, Value v) {
  return o.set_callable_return(
      std::get<ControlFlowOperationContext>(c->top().get_context()), v);
}

template <typename O>
static std::string _get_callback_key(Continuation *c, const O &o) {
  return "N/A";
}

template <CallbackOperationConcept O>
static std::string _get_callback_key(Continuation *c, const O &o) {
  return o.callback_key(
      std::get<CallbackOperationContext>(c->top().get_context()));
}

template <typename O>
static void _set_callback_called(Continuation *c, const O &o) {}

template <CallbackOperationConcept O>
static void _set_callback_called(Continuation *c, const O &o) {
  o.set_callback_called(
      std::get<CallbackOperationContext>(c->top().get_context()));
}

template <typename O>
static void _set_callback_return(Continuation *c, const O &o, Value v) {}

template <CallbackOperationConcept O>
static void _set_callback_return(Continuation *c, const O &o, Value v) {
  o.set_callback_return(
      std::get<CallbackOperationContext>(c->top().get_context()), v);
}

template <typename O>
static size_t _handle_read(Continuation *c, const O &o, std::string_view s) {
  return 0;
}

template <InputOutputOperationConcept O>
static size_t _handle_read(Continuation *c, const O &o, std::string_view s) {
  return o.handle_read(
      std::get<InputOutputOperationContext>(c->top().get_context()), s);
}

template <typename O>
static std::string_view _get_write_buffer(Continuation *c, const O &o) {
  return std::string_view();
}

template <InputOutputOperationConcept O>
static std::string_view _get_write_buffer(Continuation *c, const O &o) {
  return o.get_write_buffer(
      std::get<InputOutputOperationContext>(c->top().get_context()));
}

template <typename O>
static size_t _handle_write(Continuation *c, const O &o, size_t s) {
  return 0;
}

template <InputOutputOperationConcept O>
static size_t _handle_write(Continuation *c, const O &o, size_t s) {
  return o.handle_write(
      std::get<InputOutputOperationContext>(c->top().get_context()), s);
}

static ContinuationState map_result_to_state(Value v) {
  return ContinuationState::Ready;
}

static ContinuationState map_result_to_state(ReasonForBlockedOperation r) {
  return ContinuationState::Blocked;
}

Continuation::Continuation(std::shared_ptr<const OpTree> ot,
                           std::shared_ptr<LexicalPad> p) {
  optree = ot;
  pad = p;
  stack.push(ExecutionStackFrame(optree->root, pad));
}

ExecutionStackFrame &Continuation::top() { return stack.top(); }

ContinuationState Continuation::result_to_state() {
  return std::visit([](auto &r) { return map_result_to_state(r); }, result);
}

ContinuationState Continuation::prepare() {
  if (stack.size()) {
    while (!stack.top().has_arguments_ready()) {
      stack.push(
          ExecutionStackFrame(stack.top().next_op(), stack.top().get_pad()));
    }
    return ContinuationState::Ready;
  } else {
    return ContinuationState::Exited;
  }
}

std::shared_ptr<LexicalPad> Continuation::get_pad() { return pad; }

ContinuationState Continuation::step() {
  state = prepare();
  if (state == ContinuationState::Ready) {
    result = stack.top().execute();
    state = result_to_state();
    if (state == ContinuationState::Ready) {
      stack.pop();
      if (stack.size()) {
        stack.top().push_back(std::get<Value>(result));
      } else {
        state = ContinuationState::Exited;
      }
    }
  }
  return state;
}

OperationResult Continuation::get_result() { return result; }

std::string Continuation::get_callback_key() {
  return std::visit([this](auto &o) { return _get_callback_key(this, o); },
                    stack.top().get_operation());
}

void Continuation::set_callback_called() {
  std::visit([this](auto &o) { return _set_callback_called(this, o); },
             stack.top().get_operation());
}

const std::vector<Value> &Continuation::get_callback_arguments() {
  return *(stack.top().get_accumulator());
}

void Continuation::set_callback_return(Value v) {
  std::visit([this, v](auto &o) { _set_callback_return(this, o, v); },
             stack.top().get_operation());
}

size_t Continuation::handle_read(std::string_view in) {
  return std::visit([this, in](auto &o) { return _handle_read(this, o, in); },
                    stack.top().get_operation());
}

std::string_view Continuation::get_write_buffer() {
  return std::visit([this](auto &o) { return _get_write_buffer(this, o); },
                    stack.top().get_operation());
}

size_t Continuation::handle_write(size_t s) {
  return std::visit([this, s](auto &o) { return _handle_write(this, o, s); },
                    stack.top().get_operation());
}

Value Continuation::get_callable() {
  return std::visit([this](auto &o) { return _get_callable(this, o); },
                    stack.top().get_operation());
}

std::shared_ptr<std::vector<Value>> Continuation::get_argument_list() {
  return std::visit([this](auto &o) { return _get_argument_list(this, o); },
                    stack.top().get_operation());
}

void Continuation::set_callable_invoked() {
  std::visit([this](auto &o) { _set_callable_invoked(this, o); },
             stack.top().get_operation());
}

void Continuation::set_callable_return(Value v) {
  result = v;
  std::visit([this, &v](auto &o) { _set_callable_return(this, o, v); },
             stack.top().get_operation());
}

}; // namespace networkprotocoldsl
