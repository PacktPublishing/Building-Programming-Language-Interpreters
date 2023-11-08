#include <networkprotocoldsl/operation/eq.hpp>

namespace networkprotocoldsl::operation {

static Value _eq(int32_t lhs, int32_t rhs) { return (bool)(rhs == lhs); }

static Value _eq(int32_t lhs, value::Callable rhs) {
  return value::RuntimeError::TypeError;
}

static Value _eq(int32_t lhs, value::RuntimeError rhs) { return rhs; }

static Value _eq(int32_t lhs, Value rhs) {
  return std::visit([&lhs](auto rhs_v) -> Value { return _eq(lhs, rhs_v); },
                    rhs);
}

static Value _eq(value::Callable lhs, Value rhs) {
  return value::RuntimeError::TypeError;
}

static Value _eq(value::RuntimeError lhs, Value rhs) { return rhs; }

Value Eq::operator()(Arguments a) const {
  return std::visit([&a](auto lhs) { return _eq(lhs, std::get<1>(a)); },
                    std::get<0>(a));
}

} // namespace networkprotocoldsl::operation
