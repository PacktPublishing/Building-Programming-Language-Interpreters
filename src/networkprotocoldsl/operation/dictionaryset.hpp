#ifndef NETWORKPROTOCOLDSL_OPERATION_DICTIONARYSET_HPP
#define NETWORKPROTOCOLDSL_OPERATION_DICTIONARYSET_HPP

#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>

namespace networkprotocoldsl::operation {

struct DictionarySet {
private:
  Value set(const value::Dictionary &dict, const auto &value) const {
    auto new_dict = std::make_shared<value::Dictionary::Type>();
    for (const auto &[k, v] : *dict.members) {
      if (k != key) {
        new_dict->emplace(k, v);
      }
    }
    new_dict->emplace(key, value);
    return value::Dictionary(new_dict);
  }
  Value set(const value::Dictionary &dict,
            const value::RuntimeError &err) const {
    return err;
  }
  Value set(const value::RuntimeError &err, const auto &) const { return err; }
  Value set(const auto &, const auto &) const {
    return value::RuntimeError::TypeError;
  }

public:
  std::string key;
  Value value;
  DictionarySet(const std::string &key) : key(key) {}

  using Arguments = std::tuple<Value, Value>;

  Value operator()(const Arguments &args) const {
    return std::visit(
        [&](auto &&dict, auto &&value) -> Value { return set(dict, value); },
        std::get<0>(args), std::get<1>(args));
  }

  std::string stringify() const {
    return "DictionarySet(key: \"" + key + "\")";
  }
};

static_assert(InterpretedOperationConcept<DictionarySet>,
              "DictionarySet must conform to InterpretedOperationConcept");

} // namespace networkprotocoldsl::operation

#endif // NETWORKPROTOCOLDSL_OPERATION_DICTIONARYSET_HPP
