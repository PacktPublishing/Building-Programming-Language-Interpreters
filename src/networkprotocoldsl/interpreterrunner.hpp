#ifndef INCLUDED_NETWORKPROTOCOLDSL_INTERPRETERRUNNER_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_INTERPRETERRUNNER_HPP

#include <networkprotocoldsl/interpretercollectionmanager.hpp>
#include <networkprotocoldsl/value.hpp>

#include <future>

namespace networkprotocoldsl {

struct InterpreterRunner {
  using callback_function = std::function<Value(const std::vector<Value> &)>;
  using callback_map = std::unordered_map<std::string, callback_function>;
  callback_map callbacks;
  std::atomic<bool> exit_when_done = false;
  void interpreter_loop(InterpreterCollectionManager &mgr);
  void callback_loop(InterpreterCollectionManager &mgr);
};

} // namespace networkprotocoldsl

#endif