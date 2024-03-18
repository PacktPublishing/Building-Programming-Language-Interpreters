#include <networkprotocoldsl/operation/inttoascii.hpp>

#include <memory>
#include <sstream>

namespace networkprotocoldsl::operation {

static Value _inttoascii(int32_t v) {
  std::ostringstream os;
  os << v;
  return value::Octets{
      std::make_shared<const std::string>(std::string(os.str()))};
}

static Value _inttoascii(auto v) { return value::RuntimeError::TypeError; }

Value IntToAscii::operator()(Arguments a) const {
  return std::visit([](auto in) { return _inttoascii(in); }, std::get<0>(a));
}

} // namespace networkprotocoldsl::operation
