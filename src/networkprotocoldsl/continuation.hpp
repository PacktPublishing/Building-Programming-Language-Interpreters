#ifndef INCLUDED_NETWORKPROTOCOLDSL_CONTINUATION_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_CONTINUATION_HPP

#include <networkprotocoldsl/executionstackframe.hpp>

#include <stack>

namespace networkprotocoldsl {

/**
 * Represents the state of the execution consider external factors,
 * such as waiting for a callback to be evaluated.
 */
enum class ContinuationState { MissingArguments, Ready, Blocked, Exited };

/**
 * A continuation represents the state of a single thread of execution
 * within a intepreter. The interpreter may have many of those
 * running.
 */
class Continuation {
  std::stack<ExecutionStackFrame> stack;
  ContinuationState state = ContinuationState::MissingArguments;
  OperationResult result;

public:
  Continuation(const OpTreeNode &o);

  ExecutionStackFrame &top();

  ContinuationState result_to_state();

  ContinuationState step();

  OperationResult get_result();

  std::string get_callback_key();

  void set_callback_called();

  const std::vector<Value> &get_callback_arguments();

  void set_callback_return(Value v);

  size_t handle_read(std::string_view in);

  std::string_view get_write_buffer();

  size_t handle_write(size_t s);
};

} // namespace networkprotocoldsl

#endif
