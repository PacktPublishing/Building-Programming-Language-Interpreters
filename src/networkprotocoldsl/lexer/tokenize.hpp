#ifndef INCLUDED_NETWORKPROTOCOLDSL_LEXER_TOKENIZE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_LEXER_TOKENIZE_HPP

#include <networkprotocoldsl/lexer/token.hpp>

#include <optional>
#include <vector>

namespace networkprotocoldsl::lexer {

std::optional<std::vector<Token>> tokenize(const std::string &input);

};

#endif