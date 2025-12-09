#include <networkprotocoldsl/interpretercollection.hpp>
#include <networkprotocoldsl/interpretercollectionmanager.hpp>
#include <networkprotocoldsl/interpretercontext.hpp>
#include <networkprotocoldsl/support/notificationsignal.hpp>

#include <memory>

namespace networkprotocoldsl {

const std::shared_ptr<const InterpreterCollection>
InterpreterCollectionManager::get_collection() {
  return _collection.current();
}

std::future<Value> InterpreterCollectionManager::insert_interpreter(
    int fd, InterpretedProgram program, std::optional<Value> arglist,
    void *additional_data) {
  std::shared_ptr<InterpreterContext> ctx =
      std::make_shared<InterpreterContext>(program.get_instance(arglist));
  ctx->additional_data = additional_data;
  _collection.do_transaction(
      [&fd, &ctx](std::shared_ptr<const InterpreterCollection> current)
          -> std::shared_ptr<const InterpreterCollection> {
        auto new_interpreters = current->interpreters;
        auto old_interpreter_it = new_interpreters.find(fd);
        if (old_interpreter_it != new_interpreters.end()) {
          if (old_interpreter_it->second->exited.load()) {
            new_interpreters.erase(fd);
          } else {
            throw std::runtime_error("Interpreter already exists for fd");
          }
        }
        new_interpreters.insert({fd, ctx});
        return std::make_shared<InterpreterCollection>(
            std::move(new_interpreters), current->signals);
      });
  auto signals = _collection.current()->signals;
  signals->wake_up_interpreter.notify();
  signals->wake_up_for_output.notify();
  signals->wake_up_for_input.notify();
  signals->wake_up_for_callback.notify();

  return ctx->interpreter_result.get_future();
}

void InterpreterCollectionManager::remove_interpreter(int fd) {
  _collection.do_transaction(
      [fd](std::shared_ptr<const InterpreterCollection> current)
          -> const std::shared_ptr<const InterpreterCollection> {
        auto new_interpreters = current->interpreters;
        new_interpreters.erase(fd);
        return std::make_shared<const InterpreterCollection>(
            std::move(new_interpreters), current->signals);
      });
  auto signals = _collection.current()->signals;
  signals->wake_up_interpreter.notify();
  signals->wake_up_for_output.notify();
  signals->wake_up_for_input.notify();
  signals->wake_up_for_callback.notify();
}
} // namespace networkprotocoldsl