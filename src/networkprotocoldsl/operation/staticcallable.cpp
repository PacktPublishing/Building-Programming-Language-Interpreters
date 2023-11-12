#include <networkprotocoldsl/operation/staticcallable.hpp>

namespace networkprotocoldsl::operation {

Value StaticCallable::operator()(Arguments a) const {
  return value::Callable(optree);
}

} // namespace networkprotocoldsl::operation
