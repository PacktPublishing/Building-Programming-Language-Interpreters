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
  // If we have escape sequences to handle, we need to search for both
  // the terminator and the escape sequence
  if (escape_char.has_value() && escape_sequence.has_value()) {
    size_t pos = 0;
    while (pos < in.size()) {
      // Find the earliest occurrence of terminator or escape_sequence
      auto term_pos = in.find(terminator, pos);
      auto esc_pos = in.find(*escape_sequence, pos);
      
      if (term_pos == in.npos && esc_pos == in.npos) {
        // Neither found yet - need more data
        return 0;
      }
      
      // When escape_sequence starts with terminator (e.g., "\r\n " vs "\r\n"),
      // we need to prefer the escape sequence when it fully matches
      bool prefer_escape = false;
      if (esc_pos != in.npos) {
        if (term_pos == in.npos) {
          prefer_escape = true;
        } else if (esc_pos < term_pos) {
          prefer_escape = true;
        } else if (esc_pos == term_pos) {
          // Same position - escape sequence is longer and contains terminator,
          // so if escape matches, prefer it
          prefer_escape = true;
        }
      }
      
      if (prefer_escape) {
        // Escape sequence found before terminator
        // Append data up to escape sequence, then the escape character
        ctx.buffer.append(in.begin() + pos, in.begin() + esc_pos);
        ctx.buffer.append(*escape_char);
        pos = esc_pos + escape_sequence->size();
        continue;
      }
      
      // Terminator found (and it's before any escape sequence)
      ctx.buffer.append(in.begin() + pos, in.begin() + term_pos);
      ctx.ready = true;
      return term_pos + terminator.size();
    }
    // We processed the whole input but didn't find terminator
    return 0;
  }
  
  // No escape handling - simple case
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
