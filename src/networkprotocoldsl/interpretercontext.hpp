#ifndef INCLUDED_NETWORKPROTOCOLDSL_INTERPRETERCONTEXT_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_INTERPRETERCONTEXT_HPP

#include <networkprotocoldsl/interpreter.hpp>
#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/support/mutexlockqueue.hpp>

#include <future>
#include <memory>
#include <string>
#include <utility>

namespace networkprotocoldsl {

class InterpreterResultIsNotValue : std::exception {
public:
  OperationResult result;
  InterpreterResultIsNotValue(OperationResult r) : result(r){};
};

struct InterpreterContext {
  Interpreter interpreter;
  support::MutexLockQueue<std::string> input_buffer;
  support::MutexLockQueue<std::string> output_buffer;
  support::MutexLockQueue<std::pair<std::string, const std::vector<Value>>>
      callback_request_queue;
  support::MutexLockQueue<Value> callback_response_queue;
  std::promise<Value> interpreter_result;
  void *additional_data;

  std::atomic<bool> eof = false;
  std::atomic<bool> exited = false;

  InterpreterContext(const Interpreter &interp) : interpreter(interp) {}
  InterpreterContext(Interpreter &&interp) : interpreter(std::move(interp)) {}

  InterpreterContext() = delete;
  InterpreterContext(const InterpreterContext &) = delete;
  InterpreterContext &operator=(const InterpreterContext &) = delete;
};

} // namespace networkprotocoldsl

#endif