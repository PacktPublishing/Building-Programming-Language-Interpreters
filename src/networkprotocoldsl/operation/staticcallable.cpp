#include <networkprotocoldsl/operation/staticcallable.hpp>

namespace networkprotocoldsl::operation {

Value StaticCallable::operator()(Arguments a) const {
  return value::Callable(optree, argument_names, inherits_lexical_pad);
}

} // namespace networkprotocoldsl::operation
