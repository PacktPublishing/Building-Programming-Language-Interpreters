#include <networkprotocoldsl/operation/eq.hpp>
#include <networkprotocoldsl/value.hpp>

namespace networkprotocoldsl::operation {

static Value _eq(int32_t lhs, int32_t rhs) { return (bool)(rhs == lhs); }

static Value _eq(auto &, auto &) { return value::RuntimeError::TypeError; }

static Value _eq(int32_t lhs, value::RuntimeError rhs) { return rhs; }

static Value _eq(int32_t lhs, value::ControlFlowInstruction rhs) { return rhs; }

static Value _eq(value::RuntimeError lhs, auto rhs) { return lhs; }

static Value _eq(value::ControlFlowInstruction lhs, auto rhs) { return lhs; }

Value Eq::operator()(Arguments a) const {
  return std::visit([&a](auto lhs, auto rhs) { return _eq(lhs, rhs); },
                    std::get<0>(a), std::get<1>(a));
}

} // namespace networkprotocoldsl::operation
