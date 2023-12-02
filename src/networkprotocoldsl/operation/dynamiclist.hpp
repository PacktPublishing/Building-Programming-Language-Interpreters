#ifndef INCLUDED_NETWORKINGPROTOCOLDSL_OPERATION_DYNAMICLIST_HPP
#define INCLUDED_NETWORKINGPROTOCOLDSL_OPERATION_DYNAMICLIST_HPP

#include <networkprotocoldsl/operationconcepts.hpp>

namespace networkprotocoldsl::operation {

class DynamicList {
public:
  Value operator()(std::shared_ptr<std::vector<Value>> args) const;
};
static_assert(DynamicInputOperationConcept<DynamicList>);

} // namespace networkprotocoldsl::operation

#endif