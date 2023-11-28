#include <networkprotocoldsl/lexicalpad.hpp>

namespace networkprotocoldsl {

Value LexicalPad::get(const std::string &name) {
  auto it = pad.find(name);
  if (it == pad.end()) {
    if (parent.has_value()) {
      return parent.value()->get(name);
    } else {
      return value::RuntimeError::NameError;
    }
  } else {
    return it->second;
  }
}

Value LexicalPad::set(const std::string &name, Value v) {
  auto it = pad.find(name);
  if (it == pad.end()) {
    if (parent.has_value()) {
      return parent.value()->set(name, v);
    } else {
      return value::RuntimeError::NameError;
    }
  } else {
    Value old = it->second;
    it->second = v;
    return old;
  }
}

void LexicalPad::initialize(const std::string &name, Value v) { pad[name] = v; }

void LexicalPad::initialize_global(const std::string &name, Value v) {
  if (parent.has_value()) {
    parent.value()->initialize_global(name, v);
  } else {
    pad[name] = v;
  }
}

} // namespace networkprotocoldsl
