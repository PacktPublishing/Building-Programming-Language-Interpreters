#ifndef NETWORKPROTOCOLDSL_OPERATION_HPP
#define NETWORKPROTOCOLDSL_OPERATION_HPP

#include <networkprotocoldsl/value.hpp>

#include <cstdint>
#include <tuple>
#include <variant>

namespace networkprotocoldsl {

/**
 * The operation concept allows type erasure in some places where we use
 * std::visit.
 */
template <typename OT>
concept OperationConcept = requires(OT op, OT::Arguments args) {
  {
    std::tuple_size<typename OT::Arguments>::value
    } -> std::convertible_to<std::size_t>;
  { op(args) } -> std::convertible_to<Value>;
};

namespace operation {

/**
 * An int32_t literal type.
 */
class Int32Literal {
  int32_t int32_v;

public:
  using Arguments = std::tuple<>;
  Int32Literal(int32_t v) : int32_v(v) {}
  Value operator()(Arguments a) const { return int32_v; }
};
static_assert(OperationConcept<Int32Literal>);

/**
 * Add two values.
 */
class Add {
public:
  using Arguments = std::tuple<Value, Value>;
  Value operator()(Arguments a) const {
    return std::visit(
        [&a](int32_t v1) -> Value {
          return std::visit([&v1](int32_t v2) -> Value { return v1 + v2; },
                            std::get<1>(a));
        },
        std::get<0>(a));
  }
};
static_assert(OperationConcept<Add>);

}; // namespace operation

/**
 * Operation is a variant of all known operation types.
 *
 * This allows us to implement a type-safe dispatch on all the
 * operations that may happen in the execution of the interpreted
 * code.
 */
using Operation = std::variant<operation::Int32Literal, operation::Add>;

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_OPERATION_HPP
