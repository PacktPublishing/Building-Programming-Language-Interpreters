#include <networkprotocoldsl/lexer/token.hpp>
#include <networkprotocoldsl/lexer/token/identifier.hpp>
#include <networkprotocoldsl/lexer/token/literal.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>

#include <cstdlib>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <lexertl/generator.hpp>
#include <lexertl/lookup.hpp>

namespace networkprotocoldsl::lexer {
namespace {

// Function to unescape string literals
std::string unescape_string(const std::string &str) {
  std::string result;
  result.reserve(str.size());
  for (size_t i = 0; i < str.size(); ++i) {
    if (str[i] == '\\' && i + 1 < str.size()) {
      switch (str[i + 1]) {
      case 'n':
        result += '\n';
        break;
      case 't':
        result += '\t';
        break;
      case 'r':
        result += '\r';
        break;
      case '\\':
        result += '\\';
        break;
      case '\"':
        result += '\"';
        break;
      default:
        result += str[i + 1];
        break;
      }
      ++i;
    } else {
      result += str[i];
    }
  }
  return result;
}

struct TokenRule {
  std::string_view pattern;
  std::function<void(std::vector<Token> &, const std::string_view &)> pusher;
};

#define TP_ARGS std::vector<Token> &tokens, const std::string_view &str
const std::vector<TokenRule> token_rules = {
    {"True", [](TP_ARGS) { tokens.push_back(token::literal::Boolean{true}); }},
    {"False",
     [](TP_ARGS) { tokens.push_back(token::literal::Boolean{false}); }},
    {"for", [](TP_ARGS) { tokens.push_back(token::keyword::For()); }},
    {"in", [](TP_ARGS) { tokens.push_back(token::keyword::In()); }},
    {"message", [](TP_ARGS) { tokens.push_back(token::keyword::Message()); }},
    {"parts", [](TP_ARGS) { tokens.push_back(token::keyword::Parts()); }},
    {"terminator",
     [](TP_ARGS) { tokens.push_back(token::keyword::Terminator()); }},
    {"tokens", [](TP_ARGS) { tokens.push_back(token::keyword::Tokens()); }},
    {"[0-9]+",
     [](TP_ARGS) {
       tokens.push_back(token::literal::Integer{std::atoi(str.data())});
     }},
    {"\\\"(\\\\[\"\\\\ntr]|[^\"\\\\])*\\\"",
     [](TP_ARGS) {
       tokens.push_back(token::literal::String{
           unescape_string(std::string(str.substr(1, str.length() - 2)))});
     }},
    {">",
     [](TP_ARGS) {
       tokens.push_back(token::punctuation::AngleBracketClose());
     }},
    {"<",
     [](TP_ARGS) { tokens.push_back(token::punctuation::AngleBracketOpen()); }},
    {",", [](TP_ARGS) { tokens.push_back(token::punctuation::Comma()); }},
    {"\\}",
     [](TP_ARGS) { tokens.push_back(token::punctuation::CurlyBraceClose()); }},
    {"\\{",
     [](TP_ARGS) { tokens.push_back(token::punctuation::CurlyBraceOpen()); }},
    {"=", [](TP_ARGS) { tokens.push_back(token::punctuation::Equal()); }},
    {":",
     [](TP_ARGS) {
       tokens.push_back(token::punctuation::KeyValueSeparator());
     }},
    {";",
     [](TP_ARGS) { tokens.push_back(token::punctuation::StatementEnd()); }},
    {"\\.", [](TP_ARGS) { tokens.push_back(token::punctuation::Dot()); }},
    {"\\/\\/[^\\n]*", [](TP_ARGS) {}}, // Single-line comments (// ...)
    {"[ \\t\\n]+", [](TP_ARGS) {}}, // No action for whitespace
    {"[a-zA-Z][a-zA-Z0-9_]*",
     [](TP_ARGS) { tokens.push_back(token::Identifier{std::string(str)}); }}};
#undef TP_ARGS

} // namespace

std::optional<std::vector<Token>> tokenize(const std::string &input) {
  std::vector<Token> ret;

  lexertl::rules lexer_rules;
  lexertl::state_machine sm;

  for (unsigned short i = 0; i < token_rules.size(); ++i) {
    lexer_rules.push(token_rules[i].pattern.data(), i + 1);
  }

  lexertl::generator::build(lexer_rules, sm);
  lexertl::smatch results(input.begin(), input.end());

  while (true) {
    lexertl::lookup(sm, results);
    if (results.id == 0) {
      break;
    } else if (results.id == results.npos()) {
      return std::nullopt;
    } else {
      size_t rule_id = results.id - 1; // Adjust for zero-based index
      if (rule_id < token_rules.size()) {
        token_rules[rule_id].pusher(ret, results.str());
      }
    }
  }

  return ret;
}

} // namespace networkprotocoldsl::lexer