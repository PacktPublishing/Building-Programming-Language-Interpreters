#ifndef NETWORKPROTOCOLDSL_OPERATION_READINT32NATIVE_HPP
#define NETWORKPROTOCOLDSL_OPERATION_READINT32NATIVE_HPP

#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
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
  size_t handle_read(InputOutputOperationContext &ctx,
                     std::string_view in) const;
  void handle_eof(InputOutputOperationContext &ctx) const;
  std::string_view get_write_buffer(InputOutputOperationContext &ctx) const;

  size_t handle_write(InputOutputOperationContext &ctx, size_t s) const;

  bool ready_to_evaluate(InputOutputOperationContext &ctx) const;

  std::string stringify() const { return "ReadInt32Native{}"; }
};
static_assert(InputOutputOperationConcept<ReadInt32Native>);

} // namespace operation

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_OPERATION_HPP
