#ifndef INCLUDED_NETWORKPROTOCOLDSL_OPERATION_LEXICALPADINITIALIZEGLOBAL_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_OPERATION_LEXICALPADINITIALIZEGLOBAL_HPP

#include <networkprotocoldsl/operationconcepts.hpp>

#include <tuple>

namespace networkprotocoldsl::operation {

class LexicalPadInitializeGlobal {
  std::string name;

public:
  using Arguments = std::tuple<Value>;
  LexicalPadInitializeGlobal(const std::string &n) : name(n){};
  Value operator()(Arguments args, std::shared_ptr<LexicalPad> pad) const {
    pad->initialize_global(name, std::get<0>(args));
    return std::get<0>(args);
  }
  std::string stringify() const {
    return "LexicalPadInitializeGlobal{name: \"" + name + "\"}";
  }
};
static_assert(LexicalPadOperationConcept<LexicalPadInitializeGlobal>);

} // namespace networkprotocoldsl::operation

#endif
