#include <networkprotocoldsl/operation/add.hpp>

namespace networkprotocoldsl::operation {

Value Add::operator()(Arguments a) const {
  return std::visit(
      [&a](int32_t v1) -> Value {
        return std::visit([&v1](int32_t v2) -> Value { return v1 + v2; },
                          std::get<1>(a));
      },
      std::get<0>(a));
}

} // namespace networkprotocoldsl::operation
