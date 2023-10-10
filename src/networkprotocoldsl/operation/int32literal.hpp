#ifndef NETWORKPROTOCOLDSL_OPERATION_INT32LITERAL_HPP
#define NETWORKPROTOCOLDSL_OPERATION_INT32LITERAL_HPP

#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <tuple>
#include <variant>

namespace networkprotocoldsl {

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
static_assert(InterpretedOperationConcept<Int32Literal>);

}; // namespace operation

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_OPERATION_HPP
