#include <networkprotocoldsl/operation/terminatelistifreadahead.hpp>
#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>

namespace networkprotocoldsl::operation {

OperationResult
TerminateListIfReadAhead::operator()(InputOutputOperationContext &ctx,
                                     Arguments a) const {
  if (ctx.buffer.length() == 0) {
    // nothing to compare yet.
    if (ctx.eof) {
      return value::RuntimeError::ProtocolMismatchError;
    }
    return ReasonForBlockedOperation::WaitingForRead;
  } else if (ctx.buffer.length() < terminator.length()) {
    // we don't have the complete thing yet, but if it already mismatches we
    // can just finish the operator.
    if (ctx.buffer == terminator.substr(0, ctx.buffer.length())) {
      // we match so far, but we can't be sure yet.
      if (ctx.eof) {
        return value::RuntimeError::ProtocolMismatchError;
      } else {
        return ReasonForBlockedOperation::WaitingForRead;
      }
    } else {
      // we already know it doesn't match. Return a value.
      return false;
    }
  } else {
    // we have enough length to finish the comparison.
    if (ctx.buffer.substr(0, terminator.length()) == terminator) {
      return value::ControlFlowInstruction::InterruptGenerator;
    } else {
      return false;
    }
  }
}

size_t TerminateListIfReadAhead::handle_read(InputOutputOperationContext &ctx,
                                             std::string_view in) const {
  // the read never consumes anything in this operator, just store the data for
  // executing the operation.
  ctx.buffer = in;
  if (ctx.buffer.length() >= terminator.length() &&
      ctx.buffer.substr(0, terminator.length()) == terminator) {
    return terminator.length();
  }
  return 0;
}

std::string_view TerminateListIfReadAhead::get_write_buffer(
    InputOutputOperationContext &ctx) const {
  return ctx.buffer;
}

void TerminateListIfReadAhead::handle_eof(
    InputOutputOperationContext &ctx) const {
  ctx.eof = true;
}

size_t TerminateListIfReadAhead::handle_write(InputOutputOperationContext &ctx,
                                              size_t s) const {
  return 0;
}

} // namespace networkprotocoldsl::operation
