#ifndef INCLUDED_NETWORKPROTOCOLDSL_LEXER_TOKEN_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_LEXER_TOKEN_HPP

#include <networkprotocoldsl/lexer/token/identifier.hpp>
#include <networkprotocoldsl/lexer/token/keyword.hpp>
#include <networkprotocoldsl/lexer/token/literal.hpp>
#include <networkprotocoldsl/lexer/token/punctuation.hpp>

#include <variant>

namespace networkprotocoldsl::lexer {

using Token = std::variant<
    token::Identifier, token::keyword::For, token::keyword::In,
    token::keyword::Message, token::keyword::Parts, token::keyword::Terminator,
    token::keyword::Tokens, token::literal::Integer, token::literal::String,
    token::literal::Boolean, token::punctuation::AngleBracketClose,
    token::punctuation::AngleBracketOpen, token::punctuation::Comma,
    token::punctuation::Dot, token::punctuation::CurlyBraceClose,
    token::punctuation::CurlyBraceOpen, token::punctuation::Equal,
    token::punctuation::KeyValueSeparator, token::punctuation::StatementEnd>;

}

#endif
