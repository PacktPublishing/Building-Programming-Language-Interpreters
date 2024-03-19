#include "networkprotocoldsl/operationconcepts.hpp"
#include <cstdint>
#include <networkprotocoldsl/operation/readintfromascii.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cstring>
#include <errno.h>
#include <memory>
#include <stdlib.h>
#include <string_view>

namespace networkprotocoldsl::operation {

OperationResult ReadIntFromAscii::operator()(InputOutputOperationContext &ctx,
                                             Arguments a) const {
  if (ctx.ready) {
    if (ctx.buffer == "0") {
      return 0;
    } else {
      long converted = strtol(ctx.buffer.c_str(), NULL, 10);
      if (converted == 0) {
        return value::RuntimeError::ProtocolMismatchError;
      } else {
        if (errno == ERANGE || converted > INT_MAX || converted < INT_MIN) {
          return value::RuntimeError::ProtocolMismatchError;
        } else {
          return (int32_t)converted;
        }
      }
    }
  } else {
    return ReasonForBlockedOperation::WaitingForRead;
  }
}

size_t ReadIntFromAscii::handle_read(InputOutputOperationContext &ctx,
                                     std::string_view in) const {
  size_t consumed = 0;
  auto it = in.begin();
  for (; it != in.end() && *it >= '0' && *it < '9'; it++) {
    consumed++;
  }
  if (it != in.end()) {
    ctx.buffer = std::string(in.begin(), consumed);
    ctx.ready = true;
    return consumed;
  } else {
    return 0;
  }
}

std::string_view
ReadIntFromAscii::get_write_buffer(InputOutputOperationContext &ctx) const {
  return ctx.buffer;
}

size_t ReadIntFromAscii::handle_write(InputOutputOperationContext &ctx,
                                      size_t s) const {
  return 0;
}

} // namespace networkprotocoldsl::operation
