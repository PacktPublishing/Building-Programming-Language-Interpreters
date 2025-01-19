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
  std::shared_ptr<OpTree> optree;
  std::vector<std::string> argument_names;
  bool inherits_lexical_pad;

public:
  StaticCallable(std::shared_ptr<OpTree> o)
      : optree(o), argument_names({}), inherits_lexical_pad(true) {}
  StaticCallable(std::shared_ptr<OpTree> o, const std::vector<std::string> &n,
                 bool i)
      : optree(o), argument_names(n), inherits_lexical_pad(i) {}
  using Arguments = std::tuple<>;
  Value operator()(Arguments a) const;
  std::string stringify() const;
};
static_assert(InterpretedOperationConcept<StaticCallable>);

}; // namespace operation

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_OPERATION_HPP
