#ifndef INCLUDED_NETWORKPROTOCOLDSL_OPERATIONCONCEPTS_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_OPERATIONCONCEPTS_HPP

#include <networkprotocoldsl/lexicalpad.hpp>
#include <networkprotocoldsl/value.hpp>

#include <optional>
#include <tuple>
#include <variant>

namespace networkprotocoldsl {

/**
 * This represents the reasons why an operation may not be complete
 * because of an external reason.
 */
enum class ReasonForBlockedOperation {
  WaitingForCallback,
  WaitingCallbackData,
  WaitingForRead,
  WaitingForWrite,
  WaitingForCallableInvocation,
  WaitingForCallableResult,
};

/**
 * The OperationResult type informs whether the operation coudl have
 * been executed, or if it had to be suspeneded because of some
 * specific status.
 */
using OperationResult = std::variant<Value, ReasonForBlockedOperation>;

/**
 * The Operation Concept covers the necessary interface to discover
 * whether the stack machine has processed enough arguments for the
 * operation to be executed.
 */
template <typename OT>
concept OperationConcept = requires(OT op, typename OT::Arguments args) {
  {
    std::tuple_size<typename OT::Arguments>::value
    } -> std::convertible_to<std::size_t>;
};

/**
 * Operations that take a dynamic number of arguments.
 */
template <typename OT>
concept DynamicInputOperationConcept =
    requires(OT op, std::shared_ptr<std::vector<Value>> args) {
  { op(args) } -> std::convertible_to<Value>;
};

/**
 * Interpreted operations cannot block, and must always return a value.
 */
template <typename OT>
concept InterpretedOperationConcept = requires(OT op,
                                               typename OT::Arguments args) {
  {OperationConcept<OT>};
  { op(args) } -> std::convertible_to<Value>;
};

/**
 * Callback operations have a specific context type
 */
struct CallbackOperationContext {
  std::optional<Value> value;
  bool callback_called = false;
};

/**
 * The callback operation concept offers the additional interfaces for
 * handling the status of the callback.
 */
template <typename OT>
concept CallbackOperationConcept = requires(OT op, typename OT::Arguments args,
                                            Value v,
                                            CallbackOperationContext ctx) {
  {OperationConcept<OT>};
  { op.callback_key(ctx) } -> std::convertible_to<std::string>;
  { op(ctx, args) } -> std::convertible_to<OperationResult>;
  {op.set_callback_called(ctx)};
  {op.set_callback_return(ctx, v)};
};

/**
 * IO operations need additional context for accumulating the data
 * they need to execute.
 */
struct InputOutputOperationContext {
  std::string buffer;
  std::string::iterator it;
};

/**
 * The callback operation concept offers the additional interfaces for
 * handling IO buffers.
 */
template <typename OT>
concept InputOutputOperationConcept = requires(OT op,
                                               typename OT::Arguments args,
                                               InputOutputOperationContext ctx,
                                               std::string_view sv, size_t s) {
  {OperationConcept<OT>};
  { op(ctx, args) } -> std::convertible_to<OperationResult>;
  { op.handle_read(ctx, sv) } -> std::convertible_to<std::size_t>;
  { op.get_write_buffer(ctx) } -> std::convertible_to<std::string_view>;
  {op.handle_write(ctx, s)};
};

/**
 * Callback operations have a specific context type
 */
struct ControlFlowOperationContext {
  std::optional<Value> callable;
  std::optional<Value> value;
  std::shared_ptr<const std::vector<Value>> arglist;
  bool callable_invoked = false;
};

/**
 * The callback operation concept offers the additional interfaces for
 * handling the status of the callback.
 */
template <typename OT>
concept ControlFlowOperationConcept =
    requires(OT op, typename OT::Arguments args, Value v,
             ControlFlowOperationContext ctx) {
  {OperationConcept<OT>};
  { op.get_callable(ctx) } -> std::convertible_to<Value>;
  {
    op.get_argument_list(ctx)
    } -> std::convertible_to<std::shared_ptr<const std::vector<Value>>>;
  { op(ctx, args) } -> std::convertible_to<OperationResult>;
  {op.set_callable_return(ctx, v)};
  {op.set_callable_invoked(ctx)};
};

/**
 * The lexpad operations require access to the currently active
 * lexical pad.
 */
template <typename OT>
concept LexicalPadOperationConcept = requires(OT op,
                                              typename OT::Arguments args,
                                              std::shared_ptr<LexicalPad> pad) {
  {OperationConcept<OT>};
  { op(args, pad) } -> std::convertible_to<Value>;
};

} // namespace networkprotocoldsl

#endif
