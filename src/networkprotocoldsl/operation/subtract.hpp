#ifndef NETWORKPROTOCOLDSL_OPERATION_SUBTRACT_HPP
#define NETWORKPROTOCOLDSL_OPERATION_SUBTRACT_HPP

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
 * subtract two values.
 */
class Subtract {
public:
  using Arguments = std::tuple<Value, Value>;
  Value operator()(Arguments a) const;
};
static_assert(InterpretedOperationConcept<Subtract>);

}; // namespace operation

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_OPERATION_HPP
