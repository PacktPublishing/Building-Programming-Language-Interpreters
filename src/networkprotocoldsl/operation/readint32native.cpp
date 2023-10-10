#include <networkprotocoldsl/operation/readint32native.hpp>

namespace networkprotocoldsl::operation {

OperationResult ReadInt32Native::operator()(InputOutputOperationContext &ctx,
                                            Arguments a) const {
  return ReasonForBlockedOperation::WaitingForRead;
}

size_t ReadInt32Native::handle_read(InputOutputOperationContext &ctx) const {
  return 0;
}

size_t ReadInt32Native::handle_write(InputOutputOperationContext &ctx) const {
  return 0;
}

} // namespace networkprotocoldsl::operation
