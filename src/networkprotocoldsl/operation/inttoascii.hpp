#ifndef INCLUDED_NEWORKPROTOCOLDSL_OPERATION_INTTOASCII_HPP
#define INCLUDED_NEWORKPROTOCOLDSL_OPERATION_INTTOASCII_HPP

#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>

namespace networkprotocoldsl {

namespace operation {

/**
 * Converts an integer to ascii octets.
 */
class IntToAscii {
public:
  using Arguments = std::tuple<Value>;
  Value operator()(Arguments a) const;
  std::string stringify() const { return "IntToAscii{}"; }
};
static_assert(InterpretedOperationConcept<IntToAscii>);

}; // namespace operation

} // namespace networkprotocoldsl

#endif
