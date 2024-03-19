#include <memory>
#include <networkprotocoldsl/operation/readoctetsuntilterminator.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cstring>
#include <string_view>

namespace networkprotocoldsl::operation {

OperationResult
ReadOctetsUntilTerminator::operator()(InputOutputOperationContext &ctx,
                                      Arguments a) const {
  if (ctx.ready) {
    return value::Octets{std::make_shared<const std::string>(ctx.buffer)};
  } else {
    return ReasonForBlockedOperation::WaitingForRead;
  }
}

size_t ReadOctetsUntilTerminator::handle_read(InputOutputOperationContext &ctx,
                                              std::string_view in) const {
  auto pos = in.find(terminator);
  if (pos == in.npos) {
    return 0;
  } else {
    ctx.buffer = std::string(in.begin(), pos);
    ctx.ready = true;
    return pos + 1;
  }
}

std::string_view ReadOctetsUntilTerminator::get_write_buffer(
    InputOutputOperationContext &ctx) const {
  return ctx.buffer;
}

size_t ReadOctetsUntilTerminator::handle_write(InputOutputOperationContext &ctx,
                                               size_t s) const {
  return 0;
}

} // namespace networkprotocoldsl::operation
