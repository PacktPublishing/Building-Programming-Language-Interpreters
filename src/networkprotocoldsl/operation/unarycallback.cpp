#include <networkprotocoldsl/operation/unarycallback.hpp>

namespace networkprotocoldsl::operation {

OperationResult UnaryCallback::operator()(CallbackOperationContext &ctx,
                                          Arguments a) const {
  if (!ctx.callback_called) {
    return ReasonForBlockedOperation::WaitingForCallback;
  } else if (!ctx.value.has_value()) {
    return ReasonForBlockedOperation::WaitingCallbackData;
  } else {
    return *ctx.value;
  }
}

std::string UnaryCallback::callback_key(CallbackOperationContext &ctx) const {
  return key;
}

void UnaryCallback::set_callback_return(CallbackOperationContext &ctx,
                                        Value v) const {
  ctx.value = v;
}

void UnaryCallback::set_callback_called(CallbackOperationContext &ctx) const {
  ctx.callback_called = true;
}

} // namespace networkprotocoldsl::operation
