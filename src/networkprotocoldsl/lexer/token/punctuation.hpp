#ifndef INCLUDED_NETWORKPROTOCOLDSL_LEXER_TOKEN_PUNCTUATION_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_LEXER_TOKEN_PUNCTUATION_HPP

#include <string>

namespace networkprotocoldsl::lexer::token::punctuation {

struct AngleBracketClose {
  std::string stringify() const { return ">"; }
};
struct AngleBracketOpen {
  std::string stringify() const { return "<"; }
};
struct Comma {
  std::string stringify() const { return ","; }
};
struct CurlyBraceClose {
  std::string stringify() const { return "}"; }
};
struct CurlyBraceOpen {
  std::string stringify() const { return "{"; }
};
struct Equal {
  std::string stringify() const { return "="; }
};
struct KeyValueSeparator {
  std::string stringify() const { return ":"; }
};
struct StatementEnd {
  std::string stringify() const { return ";"; }
};
struct Dot {
  std::string stringify() const { return "."; }
};

} // namespace networkprotocoldsl::lexer::token::punctuation

#endif
