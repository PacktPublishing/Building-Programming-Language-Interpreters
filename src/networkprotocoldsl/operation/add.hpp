#ifndef NETWORKPROTOCOLDSL_OPERATION_ADD_HPP
#define NETWORKPROTOCOLDSL_OPERATION_ADD_HPP

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
 * Add two values.
 */
class Add {
public:
  using Arguments = std::tuple<Value, Value>;
  Value operator()(Arguments a) const;
};
static_assert(InterpretedOperationConcept<Add>);

}; // namespace operation

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_OPERATION_HPP
