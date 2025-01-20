#ifndef NETWORKPROTOCOLDSL_VALUE_HPP
#define NETWORKPROTOCOLDSL_VALUE_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace networkprotocoldsl {

// fwd declaration to avoid cyclic inclusion.
struct OpTree;

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
  DynamicList() = default;
  explicit DynamicList(std::vector<Value> &&vals)
      : values(std::make_shared<const std::vector<Value>>(std::move(vals))) {}
  explicit DynamicList(std::initializer_list<Value> init)
      : values(std::make_shared<const std::vector<Value>>(init)) {}
  explicit DynamicList(const std::vector<Value> &vals)
      : values(std::make_shared<const std::vector<Value>>(vals)) {}
  explicit DynamicList(std::shared_ptr<const std::vector<Value>> vals)
      : values(vals) {}
  explicit DynamicList(std::shared_ptr<std::vector<Value>> vals)
      : values(std::const_pointer_cast<const std::vector<Value>>(vals)) {}
};

struct Octets {
  std::shared_ptr<const std::string> data;
  Octets() = default;
  explicit Octets(std::string &&d)
      : data(std::make_shared<const std::string>(std::move(d))) {}
  explicit Octets(const char *d)
      : data(std::make_shared<const std::string>(d)) {}
  explicit Octets(const std::string &d)
      : data(std::make_shared<const std::string>(d)) {}
  explicit Octets(std::shared_ptr<const std::string> d) : data(d) {}
};

} // namespace value

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_VALUE_HPP
