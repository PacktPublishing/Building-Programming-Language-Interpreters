#ifndef NETWORKPROTOCOLDSL_OPERATION_DICTIONARYGET_HPP
#define NETWORKPROTOCOLDSL_OPERATION_DICTIONARYGET_HPP

#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>

namespace networkprotocoldsl::operation {

struct DictionaryGet {
private:
  Value get(const value::Dictionary &dict) const {
    auto it = dict.members->find(key);
    if (it != dict.members->end()) {
      return it->second;
    }
    return value::RuntimeError::NameError;
  }

  Value get(auto &) const { return value::RuntimeError::TypeError; }

  Value get(value::RuntimeError &err) const { return err; }

public:
  std::string key;
  DictionaryGet(const std::string &key) : key(key) {}

  using Arguments = std::tuple<Value>;

  Value operator()(const Arguments &args) const {
    return std::visit([&](auto &&dict) -> Value { return get(dict); },
                      std::get<0>(args));
  }

  std::string stringify() const {
    return "DictionaryGet{key: \"" + key + "\"}";
  }
};

static_assert(InterpretedOperationConcept<DictionaryGet>,
              "DictionaryGet must conform to InterpretedOperationConcept");

} // namespace networkprotocoldsl::operation

#endif // NETWORKPROTOCOLDSL_OPERATION_DICTIONARYGET_HPP