#include <networkprotocoldsl/operation/multiply.hpp>

namespace networkprotocoldsl::operation {

static Value _multiply(int32_t lhs, int32_t rhs) { return rhs * lhs; }

static Value _multiply(int32_t lhs, auto rhs) {
  return value::RuntimeError::TypeError;
}

static Value _multiply(int32_t lhs, value::RuntimeError rhs) { return rhs; }

static Value _multiply(int32_t lhs, Value rhs) {
  return std::visit(
      [&lhs](auto rhs_v) -> Value { return _multiply(lhs, rhs_v); }, rhs);
}

static Value _multiply(value::Callable lhs, auto rhs) {
  return value::RuntimeError::TypeError;
}

static Value _multiply(value::RuntimeError lhs, auto rhs) { return rhs; }

template <typename LHS, typename RHS> static Value _multiply(LHS lhs, RHS rhs) {
  return value::RuntimeError::TypeError;
}

Value Multiply::operator()(Arguments a) const {
  return std::visit([&a](auto lhs) { return _multiply(lhs, std::get<1>(a)); },
                    std::get<0>(a));
}

} // namespace networkprotocoldsl::operation
