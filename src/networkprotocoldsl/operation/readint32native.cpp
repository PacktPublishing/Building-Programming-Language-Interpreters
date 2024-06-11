#include <networkprotocoldsl/operation/readint32native.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cstring>

namespace networkprotocoldsl::operation {

OperationResult ReadInt32Native::operator()(InputOutputOperationContext &ctx,
                                            Arguments a) const {
  if (ctx.buffer.length() < 4) {
    if (ctx.eof)
      return value::RuntimeError::ProtocolMismatchError;
    return ReasonForBlockedOperation::WaitingForRead;
  } else {
    int v = 0;
    std::memcpy(&v, ctx.buffer.c_str(), 4);
    return v;
  }
}

size_t ReadInt32Native::handle_read(InputOutputOperationContext &ctx,
                                    std::string_view in) const {
  size_t expecting = 4 - ctx.buffer.length();
  if (expecting > 0) {
    int coming = in.length();
    if (in.length() > expecting) {
      ctx.buffer.append(in, 0, expecting);
      return expecting;
    } else {
      ctx.buffer.append(in, 0, coming);
      return coming;
    }
  } else {
    return 0;
  }
}

void ReadInt32Native::handle_eof(InputOutputOperationContext &ctx) const {
  ctx.eof = true;
}

std::string_view
ReadInt32Native::get_write_buffer(InputOutputOperationContext &ctx) const {
  return ctx.buffer;
}

size_t ReadInt32Native::handle_write(InputOutputOperationContext &ctx,
                                     size_t s) const {
  return 0;
}

} // namespace networkprotocoldsl::operation
