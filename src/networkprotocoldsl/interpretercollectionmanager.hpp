#ifndef INCLUDED_NETWORKPROTOCOLDSL_INTERPRETERMANAGER_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_INTERPRETERMANAGER_HPP

#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/interpretercollection.hpp>
#include <networkprotocoldsl/support/transactionalcontainer.hpp>

#include <atomic>
#include <memory>
#include <optional>

namespace networkprotocoldsl {

class InterpreterCollectionManager {
  support::TransactionalContainer<InterpreterCollection> _collection;

public:
  const std::shared_ptr<const InterpreterCollection> get_collection();
  std::future<Value>
  insert_interpreter(int fd, InterpretedProgram program,
                     std::optional<Value> arglist = std::nullopt,
                     void *additional_data = NULL);
  void remove_interpreter(int fd);
};

} // namespace networkprotocoldsl

#endif