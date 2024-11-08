#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_TRAITS_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_TRAITS_HPP

#include <memory>
#include <variant>

#include <networkprotocoldsl/parser/support/recursiveparser.hpp>

#include <networkprotocoldsl/lexer/token.hpp>

#include <networkprotocoldsl/parser/tree/booleanliteral.hpp>
#include <networkprotocoldsl/parser/tree/identifierreference.hpp>
#include <networkprotocoldsl/parser/tree/integerliteral.hpp>
#include <networkprotocoldsl/parser/tree/message.hpp>
#include <networkprotocoldsl/parser/tree/messagedata.hpp>
#include <networkprotocoldsl/parser/tree/messageforloop.hpp>
#include <networkprotocoldsl/parser/tree/messagepart.hpp>
#include <networkprotocoldsl/parser/tree/messagesequence.hpp>
#include <networkprotocoldsl/parser/tree/protocoldescription.hpp>
#include <networkprotocoldsl/parser/tree/stringliteral.hpp>
#include <networkprotocoldsl/parser/tree/terminator.hpp>
#include <networkprotocoldsl/parser/tree/tokenpart.hpp>
#include <networkprotocoldsl/parser/tree/tokensequence.hpp>
#include <networkprotocoldsl/parser/tree/type.hpp>
#include <networkprotocoldsl/parser/tree/typeparametermap.hpp>
#include <networkprotocoldsl/parser/tree/typeparametervalue.hpp>

namespace networkprotocoldsl::parser::grammar {

namespace {

using NodeVariant =
    std::variant<std::shared_ptr<const tree::BooleanLiteral>,
                 std::shared_ptr<const tree::IdentifierReference>,
                 std::shared_ptr<const tree::IntegerLiteral>,
                 std::shared_ptr<const tree::StringLiteral>,
                 std::shared_ptr<const tree::Type>,
                 std::shared_ptr<const tree::TypeParameterPair>,
                 std::shared_ptr<const tree::TypeParameterMap>,
                 std::shared_ptr<const tree::TokenSequence>,
                 std::shared_ptr<const tree::TokenPart>,
                 std::shared_ptr<const tree::Terminator>,
                 std::shared_ptr<const tree::ProtocolDescription>,
                 std::shared_ptr<const tree::MessageSequence>,
                 std::shared_ptr<const tree::MessagePart>,
                 std::shared_ptr<const tree::MessageForLoop>,
                 std::shared_ptr<const tree::MessageDataPair>,
                 std::shared_ptr<const tree::MessageData>,
                 std::shared_ptr<const tree::Message>>;

using TokenIterator = std::vector<lexer::Token>::const_iterator;

} // namespace

using ParseTraits = support::ParseStateTraits<TokenIterator, NodeVariant>;

} // namespace networkprotocoldsl::parser::grammar

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_TRAITS_HPP