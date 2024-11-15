#ifndef INCLUDED_NETWORKPROTOCOLDSL_LEXER_TOKEN_KEYWORD_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_LEXER_TOKEN_KEYWORD_HPP

#include <string>

namespace networkprotocoldsl::lexer::token::keyword {

struct For {
  std::string stringify() const { return "for"; }
};
struct In {
  std::string stringify() const { return "in"; }
};
struct Message {
  std::string stringify() const { return "message"; }
};
struct Parts {
  std::string stringify() const { return "parts"; }
};
struct Terminator {
  std::string stringify() const { return "terminator"; }
};
struct Tokens {
  std::string stringify() const { return "tokens"; }
};

} // namespace networkprotocoldsl::lexer::token::keyword

#endif
