#ifndef NETWORKPROTOCOLDSL_OPERATION_LESSEREQUAL_HPP
#define NETWORKPROTOCOLDSL_OPERATION_LESSEREQUAL_HPP

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
 * lesser equal of two values.
 */
class LesserEqual {
public:
  using Arguments = std::tuple<Value, Value>;
  Value operator()(Arguments a) const;
};
static_assert(InterpretedOperationConcept<LesserEqual>);

}; // namespace operation

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_OPERATION_HPP
