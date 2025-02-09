#include <networkprotocoldsl/interpretedprogram.hpp>

#include <networkprotocoldsl/generate.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>
#include <networkprotocoldsl/parser/parse.hpp>
#include <networkprotocoldsl/sema/analyze.hpp>

#include <fstream>

namespace networkprotocoldsl {

std::optional<std::shared_ptr<const sema::ast::Protocol>>
parse_protocol(const std::string &sf) {
  auto fstream = std::ifstream(sf);
  auto content = std::string(std::istreambuf_iterator<char>(fstream),
                             std::istreambuf_iterator<char>());
  auto maybe_tokens = lexer::tokenize(content);
  if (!maybe_tokens.has_value())
    return std::nullopt;
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto maybe_ast = parser::parse(tokens);
  if (!maybe_ast.has_value())
    return std::nullopt;
  return sema::analyze(maybe_ast.value());
}

std::optional<InterpretedProgram>
InterpretedProgram::generate_client(const std::string &sf) {
  auto maybe_protocol = parse_protocol(sf);
  if (!maybe_protocol.has_value())
    return std::nullopt;
  auto maybe_client = generate::client(maybe_protocol.value());
  if (!maybe_client.has_value())
    return std::nullopt;
  return InterpretedProgram(maybe_client.value());
}

std::optional<InterpretedProgram>
InterpretedProgram::generate_server(const std::string &sf) {
  auto maybe_protocol = parse_protocol(sf);
  if (!maybe_protocol.has_value())
    return std::nullopt;
  auto maybe_server = generate::server(maybe_protocol.value());
  if (!maybe_server.has_value())
    return std::nullopt;
  return InterpretedProgram(maybe_server.value());
}

} // namespace networkprotocoldsl