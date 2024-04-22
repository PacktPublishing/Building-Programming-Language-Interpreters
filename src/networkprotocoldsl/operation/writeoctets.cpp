#include <networkprotocoldsl/operation/writeoctets.hpp>
#include <networkprotocoldsl/value.hpp>

#include <networkprotocoldsl/operationconcepts.hpp>

#include <cstring>

namespace networkprotocoldsl::operation {

static OperationResult _execute_operation(InputOutputOperationContext &ctx,
                                          value::Octets &oct) {
  if (oct.data->length() == 0) {
    return 0;
  } else if (ctx.buffer.length() == 0) {
    ctx.buffer = *(oct.data);
    ctx.it = ctx.buffer.begin();
  }
  if (ctx.it != ctx.buffer.end()) {
    return ReasonForBlockedOperation::WaitingForWrite;
  } else {
    return 0;
  }
}

static OperationResult _execute_operation(InputOutputOperationContext &ctx,
                                          value::RuntimeError v) {
  return v;
}

static OperationResult _execute_operation(InputOutputOperationContext &ctx,
                                          value::ControlFlowInstruction v) {
  return v;
}

static OperationResult _execute_operation(InputOutputOperationContext &ctx,
                                          auto unknown) {
  return value::RuntimeError::TypeError;
}

OperationResult WriteOctets::operator()(InputOutputOperationContext &ctx,
                                        Arguments a) const {
  return std::visit([&ctx](auto arg) { return _execute_operation(ctx, arg); },
                    std::get<0>(a));
}

size_t WriteOctets::handle_read(InputOutputOperationContext &ctx,
                                std::string_view in) const {
  return 0;
}

std::string_view
WriteOctets::get_write_buffer(InputOutputOperationContext &ctx) const {
  return std::string_view(ctx.it, ctx.buffer.end());
}

size_t WriteOctets::handle_write(InputOutputOperationContext &ctx,
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
