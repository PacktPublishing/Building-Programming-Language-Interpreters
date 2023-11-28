#ifndef INCLUDED_NETWORKPROTOCOLDSL_OPERATION_LEXICALPADGET_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_OPERATION_LEXICALPADGET_HPP

#include <networkprotocoldsl/operationconcepts.hpp>

#include <tuple>

namespace networkprotocoldsl::operation {

class LexicalPadGet {
  std::string name;

public:
  using Arguments = std::tuple<>;
  LexicalPadGet(const std::string &n) : name(n){};
  Value operator()(Arguments args, std::shared_ptr<LexicalPad> pad) const {
    return pad->get(name);
  }
};
static_assert(LexicalPadOperationConcept<LexicalPadGet>);

} // namespace networkprotocoldsl::operation

#endif
