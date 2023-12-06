#ifndef NETWORKPROTOCOLDSL_OPERATION_MULTIPLY_HPP
#define NETWORKPROTOCOLDSL_OPERATION_MULTIPLY_HPP

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
 * multiply two values.
 */
class Multiply {
public:
  using Arguments = std::tuple<Value, Value>;
  Value operator()(Arguments a) const;
};
static_assert(InterpretedOperationConcept<Multiply>);

}; // namespace operation

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_OPERATION_HPP
