#ifndef INCLUDED_NETWORKPROTOCOLDSL_INTERPRETERRUNNER_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_INTERPRETERRUNNER_HPP

#include <networkprotocoldsl/interpretercollectionmanager.hpp>
#include <networkprotocoldsl/value.hpp>

#include <future>

namespace networkprotocoldsl {

struct InterpreterRunner {
  using callback_function = Value (*)(const std::vector<Value> &);
  std::unordered_map<std::string, callback_function> callbacks;
  std::atomic<bool> exit_when_done = false;
  void interpreter_loop(InterpreterCollectionManager &mgr);
  void callback_loop(InterpreterCollectionManager &mgr);
};

} // namespace networkprotocoldsl

#endif