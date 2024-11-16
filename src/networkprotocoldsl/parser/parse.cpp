#include <networkprotocoldsl/parser/grammar/protocoldescription.hpp>
#include <networkprotocoldsl/parser/parse.hpp>

namespace networkprotocoldsl::parser {

std::optional<std::shared_ptr<const tree::ProtocolDescription>>
parse(const std::vector<lexer::Token> &tokens) {
  auto result =
      grammar::ProtocolDescription::parse(tokens.cbegin(), tokens.cend());
  if (result.begin != tokens.cend()) {
    return std::nullopt;
  }
  if (result.node.has_value()) {
    return std::get<std::shared_ptr<const tree::ProtocolDescription>>(
        result.node.value());
  } else {
    return std::nullopt;
  }
}

} // namespace networkprotocoldsl::parser