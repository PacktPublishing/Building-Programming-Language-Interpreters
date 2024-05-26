#include <networkprotocoldsl/operation/multiply.hpp>
#include <networkprotocoldsl/value.hpp>

namespace networkprotocoldsl::operation {

static Value _multiply(int32_t lhs, int32_t rhs) { return rhs * lhs; }

static Value _multiply(int32_t lhs, auto rhs) {
  return value::RuntimeError::TypeError;
}

static Value _multiply(int32_t lhs, value::RuntimeError rhs) { return rhs; }

static Value _multiply(int32_t lhs, value::ControlFlowInstruction rhs) {
  return rhs;
}

static Value _multiply(int32_t lhs, Value rhs) {
  return std::visit(
      [&lhs](auto rhs_v) -> Value { return _multiply(lhs, rhs_v); }, rhs);
}

static Value _multiply(value::Callable lhs, auto rhs) {
  return value::RuntimeError::TypeError;
}

static Value _multiply(value::RuntimeError lhs, auto rhs) { return lhs; }

static Value _multiply(value::ControlFlowInstruction lhs, auto rhs) {
  return lhs;
}

template <typename LHS, typename RHS> static Value _multiply(LHS lhs, RHS rhs) {
  return value::RuntimeError::TypeError;
}

Value Multiply::operator()(Arguments a) const {
  return std::visit([&a](auto lhs) { return _multiply(lhs, std::get<1>(a)); },
                    std::get<0>(a));
}

} // namespace networkprotocoldsl::operation
