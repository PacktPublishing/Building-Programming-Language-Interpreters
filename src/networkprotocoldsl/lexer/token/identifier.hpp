#ifndef INCLUDED_NETWORKPROTOCOLDSL_LEXER_TOKEN_IDENTIFIER_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_LEXER_TOKEN_IDENTIFIER_HPP

#include <string>

namespace networkprotocoldsl::lexer::token {

struct Identifier {
  std::string name;
};

} // namespace networkprotocoldsl::lexer::token

#endif
