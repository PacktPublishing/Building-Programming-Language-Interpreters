#ifndef INCLUDED_NETWORKPROTOCOLDSL_OPERATION_WRITEOCTETS_H
#define INCLUDED_NETWORKPROTOCOLDSL_OPERATION_WRITEOCTETS_H

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

class WriteOctets {
public:
  using Arguments = std::tuple<Value>;
  WriteOctets() {}

  OperationResult operator()(InputOutputOperationContext &ctx,
                             Arguments a) const;
  size_t handle_read(InputOutputOperationContext &ctx,
                     std::string_view in) const;

  std::string_view get_write_buffer(InputOutputOperationContext &ctx) const;
  void handle_eof(InputOutputOperationContext &ctx) const;

  size_t handle_write(InputOutputOperationContext &ctx, size_t s) const;

  std::string stringify() const { return "WriteOctets{}"; }
};
static_assert(InputOutputOperationConcept<WriteOctets>);

} // namespace operation

} // namespace networkprotocoldsl

#endif