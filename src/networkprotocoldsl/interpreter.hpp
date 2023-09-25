#ifndef NETWORKPROTOCOLDSL_INTERPRETER_HPP
#define NETWORKPROTOCOLDSL_INTERPRETER_HPP

#include <networkprotocoldsl/operation.hpp>
#include <networkprotocoldsl/optree.hpp>

#include <cassert>
#include <memory>
#include <stack>

namespace networkprotocoldsl {

/**
 * Represents the state of the execution consider external factors,
 * such as waiting for a callback to be evaluated.
 */
enum class ExecutionStackFrameState {
  MissingArguments,
  WaitingForCallback,
  WaitingCallbackData,
  Ready,
  Exited
};

/**
 * The execution frame points to a specific operation and the
 * accumulation of inputs for that operation. The actual operation can
 * only happen when the right number of inputs has been provided.
 */
class ExecutionStackFrame {
  const OpTreeNode &optreenode;
  std::vector<Value> accumulator;
  std::optional<std::string> callback_key;
  bool callback_called = false;

  template <std::size_t... Indices>
  auto make_argument_tuple(const std::vector<Value> &v,
                           std::index_sequence<Indices...>) {
    return std::make_tuple(v[Indices]...);
  }

  template <OperationConcept O>
  ExecutionStackFrameState is_specific_operation_ready(const O &o) {
    if (accumulator.size() < std::tuple_size<typename O::Arguments>::value) {
      return ExecutionStackFrameState::MissingArguments;
    } else {
      callback_key = o.callback_key();
      if (callback_key.has_value()) {
        if (callback_called) {
          return ExecutionStackFrameState::WaitingCallbackData;
        } else {
          return ExecutionStackFrameState::WaitingForCallback;
        }
      } else {
        return ExecutionStackFrameState::Ready;
      }
    }
  }

  template <OperationConcept O> Value execute_specific_operation(const O &o) {
    typename O::Arguments args(make_argument_tuple(
        accumulator, std::make_index_sequence<
                         std::tuple_size<typename O::Arguments>::value>()));
    assert(!callback_key.has_value());
    return o(args);
  }

public:
  ExecutionStackFrame(const OpTreeNode &o) : optreenode(o) {}

  ExecutionStackFrameState is_ready() {
    return std::visit(
        [this](auto &o) { return is_specific_operation_ready(o); },
        optreenode.operation);
  }

  Value execute() {
    return std::visit([this](auto &o) { return execute_specific_operation(o); },
                      optreenode.operation);
  }

  void push_back(Value v) { accumulator.push_back(v); }

  const OpTreeNode &next_op() {
    assert(is_ready() == ExecutionStackFrameState::MissingArguments);
    return optreenode.children[accumulator.size()];
  }

  std::optional<std::string> get_callback_key() { return callback_key; }

  void set_callback_called() { callback_called = true; }
};

/**
 * A continuation represents the state of a single thread of execution
 * within a intepreter. The interpreter may have many of those
 * running.
 */
class Continuation {
  std::stack<ExecutionStackFrame> stack;
  std::optional<Value> result;

public:
  Continuation(const OpTreeNode &o) { stack.push(ExecutionStackFrame(o)); }

  ExecutionStackFrameState step() {
    if (stack.size()) {
      ExecutionStackFrameState state;
      while ((state = stack.top().is_ready()) ==
             ExecutionStackFrameState::MissingArguments) {
        stack.push(stack.top().next_op());
      }
      if (state == ExecutionStackFrameState::Ready) {
        result = stack.top().execute();
        stack.pop();
        if (stack.size()) {
          stack.top().push_back(*result);
          return stack.top().is_ready();
        } else {
          return ExecutionStackFrameState::Exited;
        }
      } else {
        return state;
      }
    } else {
      return ExecutionStackFrameState::Exited;
    }
  }

  std::optional<Value> get_result() { return result; }

  std::optional<std::string> get_callback_key() {
    assert(stack.size() > 0);
    return stack.top().get_callback_key();
  }

  void set_callback_called() {
    assert(stack.size() > 0);
    stack.top().set_callback_called();
  }

  void set_callback_return(Value v) {
    assert(stack.size() > 0);
    assert(stack.top().get_callback_key().has_value());
    result = v;
    stack.pop();
    if (stack.size()) {
      stack.top().push_back(v);
    }
  }
};

/***
 * The InterpretedProgram object represents the usage of a single program.
 *
 * Many instances of an interpreter can be spawned for a
 * program. This is where the parsing of the programming language
 * will take place, and from which a new instance can be
 * instantiated to deal with a specific connection.
 */
class Interpreter {
  std::shared_ptr<const OpTree> optree;
  Continuation continuation;

public:
  /***
   * An interpreter is constructed from an already parsed program
   * to be executed in the context of a given socket.
   */
  Interpreter(std::shared_ptr<const OpTree> o)
      : optree(o), continuation(o->root){};

  ExecutionStackFrameState step() { return continuation.step(); }

  std::optional<Value> get_result() { return continuation.get_result(); }

  std::optional<std::string> get_callback_key() {
    return continuation.get_callback_key();
  }

  void set_callback_called() { continuation.set_callback_called(); }

  void set_callback_return(Value v) { continuation.set_callback_return(v); }
};

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_INTERPRETER_HPP
