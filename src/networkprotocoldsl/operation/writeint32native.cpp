#include <networkprotocoldsl/operation/writeint32native.hpp>

#include <networkprotocoldsl/operationconcepts.hpp>

#include <cstring>

namespace networkprotocoldsl::operation {

OperationResult WriteInt32Native::operator()(InputOutputOperationContext &ctx,
                                             Arguments a) const {
  if (ctx.buffer.length() == 0) {
    int v = std::get<int32_t>(std::get<0>(a));
    char b[4] = {0, 0, 0, 0};
    std::memcpy(b, &v, 4);
    ctx.buffer = {&(b[0]), &(b[4])};
    ctx.write_it = ctx.buffer.begin();
  }
  if (ctx.write_it != ctx.buffer.end()) {
    return ReasonForBlockedOperation::WaitingForWrite;
  } else {
    return 0;
  }
}

size_t WriteInt32Native::handle_read(InputOutputOperationContext &ctx,
                                     std::string_view in) const {
  return 0;
}

std::string_view
WriteInt32Native::get_write_buffer(InputOutputOperationContext &ctx) const {
  return std::string_view(ctx.write_it, ctx.buffer.end());
}

size_t WriteInt32Native::handle_write(InputOutputOperationContext &ctx,
                                      size_t s) const {
  while (s > 0 && ctx.write_it != ctx.buffer.end()) {
    s--;
    ctx.write_it++;
  }
  return 4 - s;
}

} // namespace networkprotocoldsl::operation
