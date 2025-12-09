#ifndef INCLUDED_NETWORKPROTOCOLDSL_OPERATION_TERMINATELISTIFREAD
#define INCLUDED_NETWORKPROTOCOLDSL_OPERATION_TERMINATELISTIFREAD

#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>

namespace networkprotocoldsl {

namespace operation {

class TerminateListIfReadAhead {
  std::string terminator;

public:
  using Arguments = std::tuple<>;
  TerminateListIfReadAhead(const std::string &_t) : terminator(_t) {}

  OperationResult operator()(InputOutputOperationContext &ctx,
                             Arguments a) const;
  size_t handle_read(InputOutputOperationContext &ctx,
                     std::string_view in) const;

  std::string_view get_write_buffer(InputOutputOperationContext &ctx) const;
  void handle_eof(InputOutputOperationContext &ctx) const;

  size_t handle_write(InputOutputOperationContext &ctx, size_t s) const;

  // Returns true if the operation has enough data to produce a result.
  bool ready_to_evaluate(InputOutputOperationContext &ctx) const;

  std::string stringify() const {
    return "TerminateListIfReadAhead{terminator: \"" + terminator + "\"}";
  }
};
static_assert(InputOutputOperationConcept<TerminateListIfReadAhead>);

} // namespace operation

} // namespace networkprotocoldsl

#endif