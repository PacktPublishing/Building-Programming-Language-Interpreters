#ifndef NETWORKPROTOCOLDSL_OPERATION_DICTIONARYINITIALIZE_HPP
#define NETWORKPROTOCOLDSL_OPERATION_DICTIONARYINITIALIZE_HPP

#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>

namespace networkprotocoldsl::operation {

struct DictionaryInitialize {
  DictionaryInitialize() {}

  using Arguments = std::tuple<>;

  Value operator()(const Arguments &args) const {
    return value::Dictionary{
        std::make_shared<std::unordered_map<std::string, Value>>()};
  }

  std::string stringify() const { return "DictionaryInitialize{}"; }
};

static_assert(
    InterpretedOperationConcept<DictionaryInitialize>,
    "DictionaryInitialize must conform to InterpretedOperationConcept");

} // namespace networkprotocoldsl::operation

#endif // NETWORKPROTOCOLDSL_OPERATION_DICTIONARYINITIALIZE_HPP