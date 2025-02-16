#ifndef NETWORKPROTOCOLDSL_VALUE_HPP
#define NETWORKPROTOCOLDSL_VALUE_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
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
struct Dictionary;
} // namespace value

// variant of all types possible
using Value = std::variant<bool, int32_t, value::Dictionary, value::Callable,
                           value::RuntimeError, value::ControlFlowInstruction,
                           value::DynamicList, value::Octets>;

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

struct Dictionary {
  using Type = std::unordered_map<std::string, Value>;
  std::shared_ptr<const Type> members;
  Dictionary() : members(std::make_shared<const Type>()){};
  explicit Dictionary(Type &&m)
      : members(std::make_shared<const Type>(std::move(m))) {}
  explicit Dictionary(std::initializer_list<Type::value_type> init)
      : members(std::make_shared<const Type>(init)) {}
  explicit Dictionary(const Type &m)
      : members(std::make_shared<const Type>(m)) {}
  explicit Dictionary(std::shared_ptr<const Type> m) : members(m) {}
  explicit Dictionary(std::shared_ptr<Type> m)
      : members(std::const_pointer_cast<const Type>(m)) {}
};

} // namespace value

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_VALUE_HPP
