#include <networkprotocoldsl/operation/dynamiclist.hpp>
#include <networkprotocoldsl/value.hpp>

#include <memory>

namespace networkprotocoldsl::operation {

Value DynamicList::operator()(std::shared_ptr<std::vector<Value>> args) const {
  return value::DynamicList({args});
}

} // namespace networkprotocoldsl::operation