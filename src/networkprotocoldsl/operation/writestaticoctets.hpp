#ifndef INCLUDED_NETWORKPROTOCOLDSL_OPERATION_WRITESTATICOCTETS_H
#define INCLUDED_NETWORKPROTOCOLDSL_OPERATION_WRITESTATICOCTETS_H

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

class WriteStaticOctets {
  const std::string contents;

public:
  using Arguments = std::tuple<>;
  WriteStaticOctets(const std::string &c) : contents(c) {}

  OperationResult operator()(InputOutputOperationContext &ctx,
                             Arguments a) const;
  size_t handle_read(InputOutputOperationContext &ctx,
                     std::string_view in) const;

  std::string_view get_write_buffer(InputOutputOperationContext &ctx) const;
  void handle_eof(InputOutputOperationContext &ctx) const;

  size_t handle_write(InputOutputOperationContext &ctx, size_t s) const;

  bool ready_to_evaluate(InputOutputOperationContext &ctx) const {
    return true;  // Write operations don't wait for input
  }

  std::string stringify() const {
    return "WriteStaticOctets{contents: \"" + contents + "\"}";
  }
};
static_assert(InputOutputOperationConcept<WriteStaticOctets>);

} // namespace operation

} // namespace networkprotocoldsl

#endif