#include <networkprotocoldsl/operation/writestaticoctets.hpp>
#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cstring>

namespace networkprotocoldsl::operation {

OperationResult WriteStaticOctets::operator()(InputOutputOperationContext &ctx,
                                              Arguments a) const {
  if (ctx.buffer.length() == 0) {
    ctx.buffer = contents;
    ctx.it = ctx.buffer.begin();
  }
  if (ctx.it != ctx.buffer.end()) {
    return ReasonForBlockedOperation::WaitingForWrite;
  } else {
    return 0;
  }
}

size_t WriteStaticOctets::handle_read(InputOutputOperationContext &ctx,
                                      std::string_view in) const {
  return 0;
}

std::string_view
WriteStaticOctets::get_write_buffer(InputOutputOperationContext &ctx) const {
  return std::string_view(ctx.it, ctx.buffer.end());
}

size_t WriteStaticOctets::handle_write(InputOutputOperationContext &ctx,
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
