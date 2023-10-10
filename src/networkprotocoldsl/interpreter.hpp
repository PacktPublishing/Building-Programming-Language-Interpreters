#ifndef NETWORKPROTOCOLDSL_INTERPRETER_HPP
#define NETWORKPROTOCOLDSL_INTERPRETER_HPP

#include <networkprotocoldsl/continuation.hpp>

#include <cassert>
#include <memory>
#include <stack>
#include <variant>

namespace networkprotocoldsl {

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

  ContinuationState step() { return continuation.step(); }

  ContinuationState result_to_state() { return continuation.result_to_state(); }

  OperationResult get_result() { return continuation.get_result(); }

  std::string get_callback_key() { return continuation.get_callback_key(); }

  void set_callback_called() { continuation.set_callback_called(); }

  const std::vector<Value> &get_callback_arguments() {
    return continuation.get_callback_arguments();
  }

  void set_callback_return(Value v) { continuation.set_callback_return(v); }
};

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_INTERPRETER_HPP
