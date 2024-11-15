#ifndef INCLUDED_NETWORKPROTOCOLDSL_LEXER_TOKEN_LITERAL_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_LEXER_TOKEN_LITERAL_HPP

#include <sstream>
#include <string>

namespace networkprotocoldsl::lexer::token::literal {

struct Integer {
  int value;
  std::string stringify() const {
    std::stringstream ss = std::stringstream();
    ss << value;
    return ss.str();
  }
};
struct String {
  std::string value;
  std::string stringify() const {
    std::stringstream ss = std::stringstream();
    ss << "\"" << value << "\"";
    return ss.str();
  }
};
struct Boolean {
  bool value;
  std::string stringify() const {
    std::stringstream ss = std::stringstream();
    ss << value;
    return ss.str();
  }
};

} // namespace networkprotocoldsl::lexer::token::literal

#endif
