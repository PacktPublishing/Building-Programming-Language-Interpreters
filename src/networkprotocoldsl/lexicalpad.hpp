#ifndef INCLUDED_NETWORKPROTOCOLDSL_LEXICALPAD_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_LEXICALPAD_HPP

#include <networkprotocoldsl/value.hpp>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace networkprotocoldsl {

class LexicalPad {
  std::unordered_map<std::string, Value> pad;
  std::optional<std::shared_ptr<LexicalPad>> parent;

public:
  LexicalPad(std::shared_ptr<LexicalPad> _parent) : parent(_parent) {}
  LexicalPad() = default;
  ~LexicalPad() = default;
  Value get(const std::string &name);
  Value set(const std::string &name, Value v);
  void initialize(const std::string &name, Value v);
  void initialize_global(const std::string &name, Value v);
};

} // namespace networkprotocoldsl

#endif
