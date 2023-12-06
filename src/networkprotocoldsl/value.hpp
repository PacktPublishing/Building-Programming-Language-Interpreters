#ifndef NETWORKPROTOCOLDSL_VALUE_HPP
#define NETWORKPROTOCOLDSL_VALUE_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace networkprotocoldsl {

// fwd declaration to avoid cyclic inclusion.
class OpTree;

// fwd declare complex value types, to allow value types
// that contain other values.
namespace value {
  enum class RuntimeError;
  struct Callable;
  struct DynamicList;
}

// variant of all types possible
using Value = std::variant<bool, int32_t, value::Callable, value::RuntimeError, value::DynamicList>;

namespace value {

enum class RuntimeError { TypeError, NameError };

struct Callable {
  std::shared_ptr<const OpTree> tree;
  std::vector<std::string> argument_names;
  bool inherits_lexical_pad;
  Callable(std::shared_ptr<const OpTree> t)
      : tree(t), argument_names({}), inherits_lexical_pad(true) {}
  Callable(std::shared_ptr<const OpTree> t, const std::vector<std::string>& names, bool inherits)
      : tree(t), argument_names(names), inherits_lexical_pad(inherits) {}
};

struct DynamicList {
  std::shared_ptr<std::vector<Value>> values;
};

} // namespace value

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_VALUE_HPP
