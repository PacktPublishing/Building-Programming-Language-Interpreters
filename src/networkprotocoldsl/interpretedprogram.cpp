#include <fstream>
#include <networkprotocoldsl/generate.hpp>
#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>
#include <networkprotocoldsl/parser/parse.hpp>
#include <networkprotocoldsl/sema/analyze.hpp>

namespace networkprotocoldsl {

// New helper to parse protocol directly from source code.
static std::optional<std::shared_ptr<const sema::ast::Protocol>>
parse_protocol_from_source(const std::string &source) {
  auto maybe_tokens = lexer::tokenize(source);
  if (!maybe_tokens.has_value())
    return std::nullopt;
  std::vector<lexer::Token> tokens = maybe_tokens.value();
  auto maybe_ast = parser::parse(tokens);
  if (!maybe_ast.has_value())
    return std::nullopt;
  return sema::analyze(maybe_ast.value());
}

// New implementation: generate client from source code contents.
std::optional<InterpretedProgram>
InterpretedProgram::generate_client_from_source(const std::string &source) {
  auto maybe_protocol = parse_protocol_from_source(source);
  if (!maybe_protocol.has_value())
    return std::nullopt;
  auto maybe_client = generate::client(maybe_protocol.value());
  if (!maybe_client.has_value())
    return std::nullopt;
  return InterpretedProgram(maybe_client.value());
}

// New implementation: generate server from source code contents.
std::optional<InterpretedProgram>
InterpretedProgram::generate_server_from_source(const std::string &source) {
  auto maybe_protocol = parse_protocol_from_source(source);
  if (!maybe_protocol.has_value())
    return std::nullopt;
  auto maybe_server = generate::server(maybe_protocol.value());
  if (!maybe_server.has_value())
    return std::nullopt;
  return InterpretedProgram(maybe_server.value());
}

std::optional<InterpretedProgram>
InterpretedProgram::generate_client(const std::string &sf) {
  // Simplify: read file contents and delegate to generate_client_from_source
  std::ifstream fstream(sf);
  if (!fstream)
    return std::nullopt;
  std::string content((std::istreambuf_iterator<char>(fstream)),
                      std::istreambuf_iterator<char>());
  return generate_client_from_source(content);
}

std::optional<InterpretedProgram>
InterpretedProgram::generate_server(const std::string &sf) {
  // Simplify: read file contents and delegate to generate_server_from_source
  std::ifstream fstream(sf);
  if (!fstream)
    return std::nullopt;
  std::string content((std::istreambuf_iterator<char>(fstream)),
                      std::istreambuf_iterator<char>());
  return generate_server_from_source(content);
}

} // namespace networkprotocoldsl