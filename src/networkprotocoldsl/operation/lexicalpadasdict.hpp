#ifndef INCLUDED_NETWORKPROTOCOLDSL_OPERATION_LEXICALPADASDICT_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_OPERATION_LEXICALPADASDICT_HPP

#include <networkprotocoldsl/lexicalpad.hpp>
#include <networkprotocoldsl/operationconcepts.hpp>

#include <tuple>

namespace networkprotocoldsl::operation {

class LexicalPadAsDict {
public:
  using Arguments = std::tuple<>;
  Value operator()(Arguments args, std::shared_ptr<LexicalPad> pad) const {
    return pad->as_dict();
  }
  std::string stringify() const { return "LexicalPadAsDict{}"; }
};
static_assert(LexicalPadOperationConcept<LexicalPadAsDict>);

} // namespace networkprotocoldsl::operation

#endif