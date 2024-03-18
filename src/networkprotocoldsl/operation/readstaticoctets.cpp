#include <networkprotocoldsl/operation/readstaticoctets.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cstring>

namespace networkprotocoldsl::operation {

OperationResult ReadStaticOctets::operator()(InputOutputOperationContext &ctx,
                                             Arguments a) const {
  if (ctx.buffer.length() < contents.length()) {
    return ReasonForBlockedOperation::WaitingForRead;
  } else {
    if (strncmp(ctx.buffer.c_str(), contents.c_str(), contents.length()) == 0) {
      return true;
    } else {
      return value::RuntimeError::ProtocolMismatchError;
    }
  }
}

size_t ReadStaticOctets::handle_read(InputOutputOperationContext &ctx,
                                     std::string_view in) const {
  if (in.length() < contents.length()) {
    return 0;
  } else {
    ctx.buffer = std::string(in.begin(), contents.length());
    return contents.length();
  }
}

std::string_view
ReadStaticOctets::get_write_buffer(InputOutputOperationContext &ctx) const {
  return ctx.buffer;
}

size_t ReadStaticOctets::handle_write(InputOutputOperationContext &ctx,
                                      size_t s) const {
  return 0;
}

} // namespace networkprotocoldsl::operation
