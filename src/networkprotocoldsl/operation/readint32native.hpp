#ifndef NETWORKPROTOCOLDSL_OPERATION_READINT32NATIVE_HPP
#define NETWORKPROTOCOLDSL_OPERATION_READINT32NATIVE_HPP

#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <tuple>
#include <variant>

namespace networkprotocoldsl {

namespace operation {

class ReadInt32Native {
public:
  using Arguments = std::tuple<>;
  ReadInt32Native() {}

  OperationResult operator()(InputOutputOperationContext &ctx,
                             Arguments a) const;
  size_t handle_read(InputOutputOperationContext &ctx) const;
  size_t handle_write(InputOutputOperationContext &ctx) const;
};
static_assert(InputOutputOperationConcept<ReadInt32Native>);

} // namespace operation

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_OPERATION_HPP
