#include <networkprotocoldsl/operation/writeoctetswithescape.hpp>
#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cstring>

namespace networkprotocoldsl::operation {

// Helper to perform escape replacement on a string
static std::string apply_escape_replacement(const std::string &input,
                                            const std::string &escape_char,
                                            const std::string &escape_sequence) {
  std::string result;
  result.reserve(input.size()); // Reserve at least input size
  
  size_t pos = 0;
  while (pos < input.size()) {
    auto esc_pos = input.find(escape_char, pos);
    if (esc_pos == std::string::npos) {
      // No more escape chars, append rest of string
      result.append(input, pos, input.size() - pos);
      break;
    }
    // Append up to the escape char
    result.append(input, pos, esc_pos - pos);
    // Append the escape sequence instead of escape char
    result.append(escape_sequence);
    pos = esc_pos + escape_char.size();
  }
  
  return result;
}

static OperationResult _execute_operation(InputOutputOperationContext &ctx,
                                          value::Octets &oct,
                                          const std::string &escape_char,
                                          const std::string &escape_sequence) {
  if (oct.data->length() == 0) {
    return 0;
  } else if (ctx.buffer.length() == 0) {
    // Apply escape replacement when first initializing the buffer
    ctx.buffer = apply_escape_replacement(*(oct.data), escape_char, escape_sequence);
    ctx.it = ctx.buffer.begin();
  }
  if (ctx.it != ctx.buffer.end()) {
    if (ctx.eof) {
      return value::RuntimeError::ProtocolMismatchError;
    } else {
      return ReasonForBlockedOperation::WaitingForWrite;
    }
  } else {
    return 0;
  }
}

static OperationResult _execute_operation(InputOutputOperationContext &ctx,
                                          value::RuntimeError v,
                                          const std::string &,
                                          const std::string &) {
  return v;
}

static OperationResult _execute_operation(InputOutputOperationContext &ctx,
                                          value::ControlFlowInstruction v,
                                          const std::string &,
                                          const std::string &) {
  return v;
}

static OperationResult _execute_operation(InputOutputOperationContext &ctx,
                                          auto unknown,
                                          const std::string &,
                                          const std::string &) {
  return value::RuntimeError::TypeError;
}

OperationResult WriteOctetsWithEscape::operator()(InputOutputOperationContext &ctx,
                                                  Arguments a) const {
  return std::visit(
      [&ctx, this](auto arg) {
        return _execute_operation(ctx, arg, escape_char, escape_sequence);
      },
      std::get<0>(a));
}

void WriteOctetsWithEscape::handle_eof(InputOutputOperationContext &ctx) const {
  ctx.eof = true;
}

size_t WriteOctetsWithEscape::handle_read(InputOutputOperationContext &ctx,
                                          std::string_view in) const {
  return 0;
}

std::string_view
WriteOctetsWithEscape::get_write_buffer(InputOutputOperationContext &ctx) const {
  return std::string_view(ctx.it, ctx.buffer.end());
}

size_t WriteOctetsWithEscape::handle_write(InputOutputOperationContext &ctx,
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
