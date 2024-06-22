#ifndef INCLUDED_NETWORKPROTOCOLDSL_LEXER_TOKEN_LITERAL_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_LEXER_TOKEN_LITERAL_HPP

#include <string>

namespace networkprotocoldsl::lexer::token::literal {

struct Integer {
  int value;
};
struct String {
  std::string value;
};

} // namespace networkprotocoldsl::lexer::token::literal

#endif
