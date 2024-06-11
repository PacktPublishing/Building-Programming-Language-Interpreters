#include <networkprotocoldsl/operation/writeint32native.hpp>
#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cstddef>
#include <cstring>

namespace networkprotocoldsl::operation {

OperationResult WriteInt32Native::operator()(InputOutputOperationContext &ctx,
                                             Arguments a) const {
  if (ctx.buffer.length() == 0) {
    int v = std::get<int32_t>(std::get<0>(a));
    char b[4] = {0, 0, 0, 0};
    std::memcpy(b, &v, 4);
    ctx.buffer = {&(b[0]), &(b[4])};
    ctx.it = ctx.buffer.begin();
  }
  if (ctx.it != ctx.buffer.end()) {
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
  return std::string_view(ctx.it, ctx.buffer.end());
}

size_t WriteInt32Native::handle_write(InputOutputOperationContext &ctx,
                                      size_t s) const {
  size_t consumed = 0;
  while (s > 0 && ctx.it != ctx.buffer.end()) {
    s--;
    consumed++;
    ctx.it++;
  }
  return consumed;
}

} // namespace networkprotocoldsl::operation
