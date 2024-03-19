#ifndef NETWORKPROTOCOLDSL_OPERATION_READOCTETSUNTILTERMINATOR_HPP
#define NETWORKPROTOCOLDSL_OPERATION_READOCTETSUNTILTERMINATOR_HPP

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

class ReadOctetsUntilTerminator {
  const std::string terminator;

public:
  using Arguments = std::tuple<>;
  ReadOctetsUntilTerminator(const std::string &_t) : terminator(_t) {}

  OperationResult operator()(InputOutputOperationContext &ctx,
                             Arguments a) const;
  size_t handle_read(InputOutputOperationContext &ctx,
                     std::string_view in) const;

  std::string_view get_write_buffer(InputOutputOperationContext &ctx) const;

  size_t handle_write(InputOutputOperationContext &ctx, size_t s) const;
};
static_assert(InputOutputOperationConcept<ReadOctetsUntilTerminator>);

} // namespace operation

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_OPERATION_HPP
