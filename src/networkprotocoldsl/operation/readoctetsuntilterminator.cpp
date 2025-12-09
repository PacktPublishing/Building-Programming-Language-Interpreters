#include <networkprotocoldsl/operation/readoctetsuntilterminator.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cstring>
#include <memory>
#include <string_view>

namespace networkprotocoldsl::operation {

OperationResult
ReadOctetsUntilTerminator::operator()(InputOutputOperationContext &ctx,
                                      Arguments a) const {
  if (ctx.ready) {
    return value::Octets{std::make_shared<const std::string>(ctx.buffer)};
  } else if (ctx.eof) {
    return value::RuntimeError::ProtocolMismatchError;
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
    return pos + terminator.size();
  }
}

void ReadOctetsUntilTerminator::handle_eof(
    InputOutputOperationContext &ctx) const {
  ctx.eof = true;
}

std::string_view ReadOctetsUntilTerminator::get_write_buffer(
    InputOutputOperationContext &ctx) const {
  return ctx.buffer;
}

size_t ReadOctetsUntilTerminator::handle_write(InputOutputOperationContext &ctx,
                                               size_t s) const {
  return 0;
}

bool ReadOctetsUntilTerminator::ready_to_evaluate(
    InputOutputOperationContext &ctx) const {
  // Ready when we've found the terminator (ctx.ready) or reached EOF
  return ctx.ready || ctx.eof;
}

} // namespace networkprotocoldsl::operation
