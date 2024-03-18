#ifndef NETWORKPROTOCOLDSL_INTERPRETER_HPP
#define NETWORKPROTOCOLDSL_INTERPRETER_HPP

#include <networkprotocoldsl/continuation.hpp>
#include <networkprotocoldsl/lexicalpad.hpp>

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
  std::shared_ptr<LexicalPad> rootpad;
  std::stack<Continuation> continuation_stack;

public:
  /***
   * An interpreter is constructed from an already parsed program
   * to be executed in the context of a given socket.
   */
  Interpreter(std::shared_ptr<const OpTree> o, std::shared_ptr<LexicalPad> p)
      : optree(o), rootpad(p), continuation_stack({Continuation(o, rootpad)}){};

  ContinuationState step() {
    ContinuationState s = continuation_stack.top().step();
    if (s == ContinuationState::Blocked) {
      ReasonForBlockedOperation reason = std::get<ReasonForBlockedOperation>(
          continuation_stack.top().get_result());
      if (reason == ReasonForBlockedOperation::WaitingForCallableInvocation) {
        value::Callable callable =
            std::get<value::Callable>(continuation_stack.top().get_callable());
        continuation_stack.top().set_callable_invoked();
        std::shared_ptr<LexicalPad> parent_pad =
            callable.inherits_lexical_pad ? continuation_stack.top().get_pad()
                                          : rootpad;
        std::shared_ptr<LexicalPad> pad =
            std::make_shared<LexicalPad>(LexicalPad(parent_pad));
        std::shared_ptr<const std::vector<Value>> arglist =
            continuation_stack.top().get_argument_list();
        for (size_t i = 0; i < callable.argument_names.size(); i++) {
          if (i >= arglist->size()) {
            break;
          }
          pad->initialize(callable.argument_names.at(i), arglist->at(i));
        }
        continuation_stack.push(Continuation(callable.tree, pad));
        return continuation_stack.top().prepare();
      } else {
        return s;
      }
    } else if (s == ContinuationState::Exited) {
      Value r = std::get<Value>(continuation_stack.top().get_result());
      if (continuation_stack.size() > 1) {
        continuation_stack.pop();
        continuation_stack.top().set_callable_return(r);
        return continuation_stack.top().prepare();
      } else {
        return s;
      }
    } else {
      return s;
    }
  }

  ContinuationState result_to_state() {
    return continuation_stack.top().result_to_state();
  }

  OperationResult get_result() { return continuation_stack.top().get_result(); }

  std::string get_callback_key() {
    return continuation_stack.top().get_callback_key();
  }

  void set_callback_called() { continuation_stack.top().set_callback_called(); }

  const std::vector<Value> &get_callback_arguments() {
    return continuation_stack.top().get_callback_arguments();
  }

  void set_callback_return(Value v) {
    continuation_stack.top().set_callback_return(v);
  }

  size_t handle_read(std::string_view in) {
    return continuation_stack.top().handle_read(in);
  }

  std::string_view get_write_buffer() {
    return continuation_stack.top().get_write_buffer();
  }

  size_t handle_write(size_t s) {
    return continuation_stack.top().handle_write(s);
  }
};

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_INTERPRETER_HPP
