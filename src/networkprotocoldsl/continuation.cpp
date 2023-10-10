#include <networkprotocoldsl/continuation.hpp>

namespace networkprotocoldsl {

template <InterpretedOperationConcept O>
static std::string _get_callback_key(Continuation *c, const O &o) {
  return "N/A";
}

template <CallbackOperationConcept O>
static std::string _get_callback_key(Continuation *c, const O &o) {
  return o.callback_key(
      std::get<CallbackOperationContext>(c->top().get_context()));
}

template <InputOutputOperationConcept O>
static std::string _get_callback_key(Continuation *c, const O &o) {
  return "N/A";
}

template <InterpretedOperationConcept O>
static void _set_callback_called(Continuation *c, const O &o) {}

template <CallbackOperationConcept O>
static void _set_callback_called(Continuation *c, const O &o) {
  o.set_callback_called(
      std::get<CallbackOperationContext>(c->top().get_context()));
}

template <InputOutputOperationConcept O>
static void _set_callback_called(Continuation *c, const O &o) {}

template <InterpretedOperationConcept O>
static void _set_callback_return(Continuation *c, const O &o, Value v) {}

template <CallbackOperationConcept O>
static void _set_callback_return(Continuation *c, const O &o, Value v) {
  o.set_callback_return(
      std::get<CallbackOperationContext>(c->top().get_context()), v);
}

template <InputOutputOperationConcept O>
static void _set_callback_return(Continuation *c, const O &o, Value v) {}

template <InterpretedOperationConcept O>
static size_t _handle_read(Continuation *c, const O &o, std::string_view s) {
  return 0;
}

template <CallbackOperationConcept O>
static size_t _handle_read(Continuation *c, const O &o, std::string_view s) {
  return 0;
}

template <InputOutputOperationConcept O>
static size_t _handle_read(Continuation *c, const O &o, std::string_view s) {
  return o.handle_read(
      std::get<InputOutputOperationContext>(c->top().get_context()), s);
}

template <InterpretedOperationConcept O>
static std::string_view _get_write_buffer(Continuation *c, const O &o) {
  return std::string_view();
}

template <CallbackOperationConcept O>
static std::string_view _get_write_buffer(Continuation *c, const O &o) {
  return std::string_view();
}

template <InputOutputOperationConcept O>
static std::string_view _get_write_buffer(Continuation *c, const O &o) {
  return o.get_write_buffer(
      std::get<InputOutputOperationContext>(c->top().get_context()));
}

template <InterpretedOperationConcept O>
static size_t _handle_write(Continuation *c, const O &o, size_t s) {
  return 0;
}

template <CallbackOperationConcept O>
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

Continuation::Continuation(const OpTreeNode &o) {
  stack.push(ExecutionStackFrame(o));
}

ExecutionStackFrame &Continuation::top() { return stack.top(); }

ContinuationState Continuation::result_to_state() {
  return std::visit([this](auto &r) { return map_result_to_state(r); }, result);
}

ContinuationState Continuation::step() {
  if (stack.size()) {
    while (!stack.top().has_arguments_ready()) {
      stack.push(stack.top().next_op());
    }
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
    return state;
  } else {
    return ContinuationState::Exited;
  }
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
  return stack.top().get_accumulator();
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

}; // namespace networkprotocoldsl
