#ifndef INCLUDED_NETWORKPROTOCOLDSL_OPERATION_LEXICALPADSET_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_OPERATION_LEXICALPADSET_HPP

#include <networkprotocoldsl/operationconcepts.hpp>

#include <tuple>

namespace networkprotocoldsl::operation {

class LexicalPadSet {
  std::string name;

public:
  using Arguments = std::tuple<Value>;
  LexicalPadSet(const std::string &n) : name(n){};
  Value operator()(Arguments args, std::shared_ptr<LexicalPad> pad) const {
    return pad->set(name, std::get<0>(args));
  }
  std::string stringify() const {
    return "LexicalPadSet{name: \"" + name + "\"}";
  }
};
static_assert(LexicalPadOperationConcept<LexicalPadSet>);

} // namespace networkprotocoldsl::operation

#endif
