#ifndef NETWORKPROTOCOLDSL_VALUE_HPP
#define NETWORKPROTOCOLDSL_VALUE_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <variant>

namespace networkprotocoldsl {
// fwd declaration to avoid cyclic inclusion.
class OpTree;

namespace value {

enum class RuntimeError { TypeError };

struct Callable {
  std::shared_ptr<const OpTree> tree;
};
} // namespace value

/**
 * A value is a variant of different types.
 */
using Value = std::variant<int32_t, value::Callable, value::RuntimeError>;
} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_VALUE_HPP
