#include <networkprotocoldsl/operation/add.hpp>
#include <networkprotocoldsl/value.hpp>

namespace networkprotocoldsl::operation {

static Value _add(int32_t lhs, int32_t rhs) { return rhs + lhs; }

static Value _add(auto &, auto &) { return value::RuntimeError::TypeError; }

static Value _add(value::RuntimeError lhs, auto rhs) { return lhs; }

static Value _add(value::ControlFlowInstruction lhs, auto rhs) { return lhs; }

static Value _add(int32_t lhs, value::RuntimeError rhs) { return rhs; }

static Value _add(int32_t lhs, value::ControlFlowInstruction rhs) {
  return rhs;
}

Value Add::operator()(Arguments a) const {
  return std::visit([](auto lhs, auto rhs) { return _add(lhs, rhs); },
                    std::get<0>(a), std::get<1>(a));
}

} // namespace networkprotocoldsl::operation
