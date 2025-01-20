#ifndef NETWORKPROTOCOLDSL_OPERATION_UNARYCALLBACK_HPP
#define NETWORKPROTOCOLDSL_OPERATION_UNARYCALLBACK_HPP

#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <tuple>
#include <variant>

namespace networkprotocoldsl {

namespace operation {

class UnaryCallback {
  const std::string key;

public:
  UnaryCallback(std::string c) : key(c) {}
  using Arguments = std::tuple<Value>;
  OperationResult operator()(CallbackOperationContext &ctx, Arguments a) const;
  std::string callback_key(CallbackOperationContext &ctx) const;
  void set_callback_return(CallbackOperationContext &ctx, Value v) const;
  void set_callback_called(CallbackOperationContext &ctx) const;

  std::string stringify() const {
    return "UnaryCallback{key: \"" + key + "\"}";
  }
};
static_assert(CallbackOperationConcept<UnaryCallback>);

}; // namespace operation

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_OPERATION_HPP
