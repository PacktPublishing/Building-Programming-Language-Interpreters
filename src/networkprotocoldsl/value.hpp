#ifndef NETWORKPROTOCOLDSL_VALUE_HPP
#define NETWORKPROTOCOLDSL_VALUE_HPP

#include <cstdint>
#include <string>
#include <variant>

namespace networkprotocoldsl {
/**
 * A value is a variant of different types.
 */
using Value = std::variant<int32_t>;
} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_VALUE_HPP
