#ifndef NETWORKPROTOCOLDSL_OPERATION_OPSEQUENCE_HPP
#define NETWORKPROTOCOLDSL_OPERATION_OPSEQUENCE_HPP

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
 * Operation that instructs the interpreter to execute the child
 * operations one at a time.
 */
class OpSequence {
public:
  Value operator()(std::shared_ptr<std::vector<Value>> args) const;
  std::string stringify() const { return "OpSequence{}"; }
};
static_assert(DynamicInputOperationConcept<OpSequence>);

}; // namespace operation

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_OPERATION_HPP
