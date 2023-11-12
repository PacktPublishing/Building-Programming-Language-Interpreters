#ifndef NETWORKPROTOCOLDSL_OPERATION_STATICCALLABLE_HPP
#define NETWORKPROTOCOLDSL_OPERATION_STATICCALLABLE_HPP

#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <tuple>
#include <variant>

namespace networkprotocoldsl {

namespace operation {

class StaticCallable {
  const std::shared_ptr<OpTree> optree;

public:
  StaticCallable(std::shared_ptr<OpTree> o) : optree(o) {}
  using Arguments = std::tuple<>;
  Value operator()(Arguments a) const;
};
static_assert(InterpretedOperationConcept<StaticCallable>);

}; // namespace operation

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_OPERATION_HPP
