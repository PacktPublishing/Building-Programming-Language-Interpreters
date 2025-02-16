#include <iostream> // Add this include for std::cerr
#include <networkprotocoldsl/parser/grammar/protocoldescription.hpp>
#include <networkprotocoldsl/parser/parse.hpp>

namespace networkprotocoldsl::parser {

std::optional<std::shared_ptr<const tree::ProtocolDescription>>
parse(const std::vector<lexer::Token> &tokens) {
  auto result =
      grammar::ProtocolDescription::parse(tokens.cbegin(), tokens.cend());
  if (result.begin != tokens.cend()) {
    // Define a visitor lambda that uses stringify if available.
    auto tokenToStringVisitor = [](const auto &token) -> std::string {
      if constexpr (requires { token.stringify(); }) {
        return token.stringify();
      } else {
        return "<NOT_STRINGIFIABLE>";
      }
    };
    // Print to stderr the next 10 tokens starting at the current position.
    std::cerr << "Parsing stopped. Next tokens: ";
    auto it = result.begin;
    int count = 0;
    while (it != tokens.cend() && count < 10) {
      std::cerr << std::visit(tokenToStringVisitor, *it) << " ";
      ++it;
      ++count;
    }
    std::cerr << std::endl;
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