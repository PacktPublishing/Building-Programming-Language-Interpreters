#ifndef INCLUDED_NETWORKPROTOCOLDSL_OPERATIONCONCEPTS_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_OPERATIONCONCEPTS_HPP

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
  WaitingForWrite
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
};

/**
 * The callback operation concept offers the additional interfaces for
 * handling IO buffers.
 */
template <typename OT>
concept InputOutputOperationConcept = requires(
    OT op, typename OT::Arguments args, InputOutputOperationContext ctx) {
  {OperationConcept<OT>};
  { op(ctx, args) } -> std::convertible_to<OperationResult>;
  { op.handle_read(ctx) } -> std::convertible_to<std::size_t>;
  { op.handle_write(ctx) } -> std::convertible_to<std::size_t>;
};

} // namespace networkprotocoldsl

#endif
