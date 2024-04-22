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
enum class ControlFlowInstruction;
struct Callable;
struct DynamicList;
struct Octets;
} // namespace value

// variant of all types possible
using Value = std::variant<bool, int32_t, value::Callable, value::RuntimeError,
                           value::ControlFlowInstruction, value::DynamicList,
                           value::Octets>;

namespace value {

enum class RuntimeError { TypeError, NameError, ProtocolMismatchError };

enum class ControlFlowInstruction { InterruptGenerator };

struct Callable {
  std::shared_ptr<const OpTree> tree;
  std::vector<std::string> argument_names;
  bool inherits_lexical_pad;
  Callable(std::shared_ptr<const OpTree> t)
      : tree(t), argument_names({}), inherits_lexical_pad(true) {}
  Callable(std::shared_ptr<const OpTree> t,
           const std::vector<std::string> &names, bool inherits)
      : tree(t), argument_names(names), inherits_lexical_pad(inherits) {}
};

struct DynamicList {
  std::shared_ptr<const std::vector<Value>> values;
};

struct Octets {
  std::shared_ptr<const std::string> data;
};

} // namespace value

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_VALUE_HPP
