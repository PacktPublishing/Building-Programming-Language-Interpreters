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

enum class RuntimeError { TypeError, NameError };

struct Callable {
  std::shared_ptr<const OpTree> tree;
  bool inherits_lexical_pad;
  Callable(std::shared_ptr<const OpTree> t)
      : tree(t), inherits_lexical_pad(true) {}
};
} // namespace value

/**
 * A value is a variant of different types.
 */
using Value = std::variant<bool, int32_t, value::Callable, value::RuntimeError>;
} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_VALUE_HPP
