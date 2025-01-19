#ifndef NETWORKPROTOCOLDSL_OPERATION_EQ_HPP
#define NETWORKPROTOCOLDSL_OPERATION_EQ_HPP

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
 * Compares two values.
 */
class Eq {
public:
  using Arguments = std::tuple<Value, Value>;
  Value operator()(Arguments a) const;
  std::string stringify() const { return "Eq{}"; }
};
static_assert(InterpretedOperationConcept<Eq>);

}; // namespace operation

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_OPERATION_HPP
