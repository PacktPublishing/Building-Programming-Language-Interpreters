#ifndef INCLUDED_NETWORKPROTOCOLDSL_OPERATION_LEXICALPADINITIALIZE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_OPERATION_LEXICALPADINITIALIZE_HPP

#include <networkprotocoldsl/operationconcepts.hpp>

#include <tuple>

namespace networkprotocoldsl::operation {

class LexicalPadInitialize {
  std::string name;

public:
  using Arguments = std::tuple<Value>;
  LexicalPadInitialize(const std::string &n) : name(n){};
  Value operator()(Arguments args, std::shared_ptr<LexicalPad> pad) const {
    pad->initialize(name, std::get<0>(args));
    return std::get<0>(args);
  }
  std::string stringify() const {
    return "LexicalPadInitialize{name: \"" + name + "\"}";
  }
};
static_assert(LexicalPadOperationConcept<LexicalPadInitialize>);

} // namespace networkprotocoldsl::operation

#endif
