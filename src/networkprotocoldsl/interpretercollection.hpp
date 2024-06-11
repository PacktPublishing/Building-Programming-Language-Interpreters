#ifndef INCLUDED_NETWORKPROTOCOLDSL_INTERPRETERCOLLECTION_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_INTERPRETERCOLLECTION_HPP

#include <networkprotocoldsl/interpretercontext.hpp>
#include <networkprotocoldsl/support/notificationsignal.hpp>

#include <memory>
#include <unordered_map>

namespace networkprotocoldsl {

struct InterpreterSignals {
  support::NotificationSignal wake_up_interpreter;
  support::NotificationSignal wake_up_for_output;
  support::NotificationSignal wake_up_for_input;
  support::NotificationSignal wake_up_for_callback;
  InterpreterSignals()
      : wake_up_interpreter(support::NotificationSignal()),
        wake_up_for_output(support::NotificationSignal()),
        wake_up_for_input(support::NotificationSignal()),
        wake_up_for_callback(support::NotificationSignal()) {}
};

struct InterpreterCollection {
  const std::unordered_map<int, std::shared_ptr<InterpreterContext>>
      interpreters;
  const std::shared_ptr<InterpreterSignals> signals =
      std::make_shared<InterpreterSignals>();
};

} // namespace networkprotocoldsl

#endif