#include <networkprotocoldsl/operation/staticcallable.hpp>
#include <networkprotocoldsl/print_optreenode.hpp>

namespace networkprotocoldsl::operation {

Value StaticCallable::operator()(Arguments a) const {
  return value::Callable(optree, argument_names, inherits_lexical_pad);
}

std::string StaticCallable::stringify() const {
  std::ostringstream os;
  os << "StaticCallable{\n";
  os << " inherits_lexical_pad: " << std::to_string(inherits_lexical_pad)
     << "\n";
  os << " argument_names: [";
  for (size_t i = 0; i < argument_names.size(); ++i) {
    os << "\"" << argument_names[i] << "\"";
    if (i < argument_names.size() - 1) {
      os << ", ";
    }
  }
  os << "]\n optree: \n";
  print_optreenode(optree->root, os, "  │ ", "  │ ");
  os << "}";
  return os.str();
}

} // namespace networkprotocoldsl::operation
