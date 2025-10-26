#include "networkprotocoldsl/value.hpp"
#include <networkprotocoldsl/continuation.hpp>
#include <networkprotocoldsl/interpretercollection.hpp>
#include <networkprotocoldsl/interpretercollectionmanager.hpp>
#include <networkprotocoldsl/interpretercontext.hpp>
#include <networkprotocoldsl/interpreterrunner.hpp>
#include <networkprotocoldsl/operationconcepts.hpp>

#include <exception>
#include <iostream>
#include <thread>
#include <variant>

#define INTERPRETERRUNNER_DEBUG(x)
//#define INTERPRETERRUNNER_DEBUG(x) std::cerr << "InterpreterRunner[" << \
// std::this_thread::get_id() << "] " << __func__ << ": " << x << std::endl

namespace networkprotocoldsl {

namespace {

static bool handle_read(InterpreterContext &context,
                        InterpreterSignals &signals) {
  auto buffer = context.input_buffer.pop();
  if (buffer.has_value()) {
    INTERPRETERRUNNER_DEBUG("handle_read: got buffer of size " << buffer->size() << ": '"
                                      << *buffer << "'");
    size_t consumed = context.interpreter.handle_read(buffer.value());
    INTERPRETERRUNNER_DEBUG("handle_read: consumed " << consumed << " bytes");
    if (consumed == 0) {
      // since the op didn't consume anything, we try to join with the next
      // buffer to see if the next read works when things get joined together
      auto nextbuffer = context.input_buffer.pop();
      if (nextbuffer.has_value()) {
        INTERPRETERRUNNER_DEBUG("handle_read: joining buffers of size " << buffer->size() << " and " << nextbuffer->size() << ": '"
                                          << *buffer << "' + '" << *nextbuffer << "'");
        auto joined = buffer.value() + nextbuffer.value();
        consumed = context.interpreter.handle_read(joined);
        INTERPRETERRUNNER_DEBUG("handle_read: after joined read, consumed " << consumed << " bytes");
        if (consumed < joined.size()) {
          context.input_buffer.push_front(joined.substr(consumed));
          INTERPRETERRUNNER_DEBUG("handle_read: pushed back unconsumed data of size " << (joined.size() - consumed) << ": '"
                                            << joined.substr(consumed) << "'");
        }
        return true;
      } else {
        context.input_buffer.push_front(buffer.value());
        INTERPRETERRUNNER_DEBUG("handle_read: pushed back original buffer of size " << buffer->size() << ": '"
                                          << buffer.value() << "'");
        return false;
      }
    } else {
      if (consumed < buffer.value().size()) {
        INTERPRETERRUNNER_DEBUG("handle_read: pushed back unconsumed data of size " << (buffer->size() - consumed) << ": '"
                                          << buffer->substr(consumed) << "'");
        context.input_buffer.push_front(buffer->substr(consumed));
      }
      return true;
    }
  } else {
    if (context.eof.load()) {
      context.interpreter.handle_eof();
      INTERPRETERRUNNER_DEBUG("handle_read: EOF reached");
      return true;
    }
    signals.wake_up_for_input.notify();
    signals.wake_up_for_output.notify();
  }
  return false;
}

bool handle_write(InterpreterContext &context, InterpreterSignals &signals) {
  if (context.eof.load()) {
    context.interpreter.handle_eof();
    signals.wake_up_for_output.notify();
    signals.wake_up_for_input.notify();
    return true;
  } else {
    auto buffer = context.interpreter.get_write_buffer();
    std::string copy(buffer);
    context.output_buffer.push_back(copy);
    context.interpreter.handle_write(buffer.size());
    signals.wake_up_for_output.notify();
    return true;
  }
}

bool handle_start_callback(InterpreterContext &context,
                           InterpreterSignals &signals) {
  context.callback_request_queue.push_back(
      std::make_pair(context.interpreter.get_callback_key(),
                     context.interpreter.get_callback_arguments()));
  context.interpreter.set_callback_called();
  signals.wake_up_for_callback.notify();
  return true;
}

bool handle_finish_callback(InterpreterContext &context,
                            InterpreterSignals &signals) {
  auto v = context.callback_response_queue.pop();
  if (v.has_value()) {
    context.interpreter.set_callback_return(v.value());
    return true;
  } else {
    return false;
  }
}

bool handle_blocked_interpreter(InterpreterContext &context,
                                InterpreterSignals &signals) {
  using namespace networkprotocoldsl;
  OperationResult r = context.interpreter.get_result();
  if (std::holds_alternative<ReasonForBlockedOperation>(r)) {
    ReasonForBlockedOperation reason = std::get<ReasonForBlockedOperation>(r);
    switch (reason) {
    case ReasonForBlockedOperation::WaitingForRead:
      INTERPRETERRUNNER_DEBUG("WaitingForRead");
      return handle_read(context, signals);
    case ReasonForBlockedOperation::WaitingForWrite:
      INTERPRETERRUNNER_DEBUG("WaitingForWrite");
      return handle_write(context, signals);
    case ReasonForBlockedOperation::WaitingForCallback:
      INTERPRETERRUNNER_DEBUG("WaitingForCallback");
      return handle_start_callback(context, signals);
    case ReasonForBlockedOperation::WaitingCallbackData:
      INTERPRETERRUNNER_DEBUG("WaitingCallbackData");
      return handle_finish_callback(context, signals);
    case ReasonForBlockedOperation::WaitingForCallableInvocation:
    case ReasonForBlockedOperation::WaitingForCallableResult:
      INTERPRETERRUNNER_DEBUG("WaitingForCallableInvocation/Result");
      // this is not really a blocker, the interpreter sorts itself
      return true;
    }
  }
  return true;
}
} // namespace

void InterpreterRunner::interpreter_loop(InterpreterCollectionManager &mgr) {
  while (true) {
    auto collection = mgr.get_collection();
    int active_interpreters = 0;
    int ready_interpreters = 0;
    for (auto &[fd, context] : collection->interpreters) {
      if (context->exited.load()) {
        continue;
      }
      auto state = context->interpreter.step();
      switch (state) {
      case ContinuationState::MissingArguments:
      case ContinuationState::Ready:
        ready_interpreters++;
        active_interpreters++;
        INTERPRETERRUNNER_DEBUG("Ready interpreter");
        break;
      case ContinuationState::Blocked:
        active_interpreters++;
        if (handle_blocked_interpreter(*context, *collection->signals)) {
          ready_interpreters++;
        }
        INTERPRETERRUNNER_DEBUG("Blocked interpreter");
        break;
      case ContinuationState::Exited:
        context->exited.store(true);
        OperationResult r = context->interpreter.get_result();
        if (std::holds_alternative<Value>(r)) {
          Value v = std::get<Value>(r);

          context->interpreter_result.set_value(v);
        } else {
          context->interpreter_result.set_exception(
              std::make_exception_ptr(InterpreterResultIsNotValue(r)));
        }
        collection->signals->wake_up_for_output.notify();
        collection->signals->wake_up_for_input.notify();
        collection->signals->wake_up_for_callback.notify();
        collection->signals->wake_up_interpreter.notify();
        INTERPRETERRUNNER_DEBUG("Exited interpreter");
        break;
      };
    }
    bool loaded_exit_when_done = exit_when_done.load();
    if (mgr.get_collection() == collection) {
      if (active_interpreters == 0) {
        if (loaded_exit_when_done) {
          collection->signals->wake_up_for_callback.notify();
          collection->signals->wake_up_for_input.notify();
          collection->signals->wake_up_for_output.notify();
          break;
        } else {
          INTERPRETERRUNNER_DEBUG("Waiting, no active interpreter");
          collection->signals->wake_up_interpreter.wait();
          INTERPRETERRUNNER_DEBUG("Woken up...");
        }
      } else {
        if (ready_interpreters == 0) {
          INTERPRETERRUNNER_DEBUG("Waiting, no ready interpreter");
          collection->signals->wake_up_interpreter.wait();
          INTERPRETERRUNNER_DEBUG("Woken up...");
        }
      }
    }
  }
}

void InterpreterRunner::callback_loop(InterpreterCollectionManager &mgr) {
  while (true) {
    int callbacks_count = 0;
    int active_interpreters = 0;
    auto collection = mgr.get_collection();
    for (auto &[fd, context] : collection->interpreters) {
      if (context->exited.load()) {
        continue;
      }
      active_interpreters++;
      auto cbdata = context->callback_request_queue.pop();
      if (cbdata.has_value()) {
        callbacks_count++;
        const auto &key = cbdata.value().first;
        const auto cb_it = callbacks.find(key);
        if (cb_it == callbacks.end()) {
          context->callback_response_queue.push_back(
              value::RuntimeError::TypeError);
        } else {
          auto fp = cb_it->second;
          context->callback_response_queue.push_back(fp(cbdata.value().second));
        }
        collection->signals->wake_up_interpreter.notify();
      }
    }
    bool loaded_exit_when_done = exit_when_done.load();
    if (mgr.get_collection() == collection) {
      if (active_interpreters == 0) {
        if (loaded_exit_when_done) {
          collection->signals->wake_up_interpreter.notify();
          collection->signals->wake_up_for_input.notify();
          collection->signals->wake_up_for_output.notify();
          break;
        } else {
          INTERPRETERRUNNER_DEBUG("Waiting, no active interpreter");
          collection->signals->wake_up_for_callback.wait();
          INTERPRETERRUNNER_DEBUG("Woken up...");
        }
      } else {
        if (callbacks_count == 0) {
          INTERPRETERRUNNER_DEBUG("Waiting, no callbacks to process");
          collection->signals->wake_up_for_callback.wait();
          INTERPRETERRUNNER_DEBUG("Woken up...");
        }
      }
    }
  }
}

} // namespace networkprotocoldsl