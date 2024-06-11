#include <networkprotocoldsl/operation/opsequence.hpp>

#include <memory>

namespace networkprotocoldsl::operation {

Value OpSequence::operator()(std::shared_ptr<std::vector<Value>> args) const {
  return args->back();
}

} // namespace networkprotocoldsl::operation