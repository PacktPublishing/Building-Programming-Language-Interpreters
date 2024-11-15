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

static int indent_level(int modify) {
  static int indent = 0;
  indent += modify;
  for (int i = 0; i < indent; ++i) {
    std::cerr << ' ';
  }
  std::cerr << '(' << indent << ") ";
  return indent;
}

static bool trace_enabled() {
  static bool r = getenv("GRAMMAR_TRACER") != nullptr;
  return r;
}

} // namespace

template <typename ParserContext> class Tracer {
public:
  static void output_tokens(const TokenIterator begin,
                            const TokenIterator end) {
    int max = 10;
    TokenIterator b = begin;
    while (max > 0 && b != end) {
      std::cerr << " " << std::visit([&](auto t) { return t.stringify(); }, *b);
      b++;
      max--;
    }
    if (b != end) {
      std::cerr << "...";
    }
  }

  using TokenIterator = ParserContext::TokenIterator;
  template <typename... Args>
  static void trace_start(const char *attempt_type, const TokenIterator &begin,
                          const TokenIterator &end, Args... args) {
    if (!trace_enabled())
      return;
    indent_level(1);
    std::cerr << "> " << attempt_type << " " << ParserContext::name << " ("
              << sizeof...(args) << "):";
    output_tokens(begin, end);
    std::cerr << std::endl;
  }
  template <typename... Args>
  static void trace_success(const TokenIterator &begin,
                            const TokenIterator &end, Args... args) {
    if (!trace_enabled())
      return;
    indent_level(-1);
    std::cerr << "< " << ParserContext::name << " [SUCCESS]";
    output_tokens(begin, end);
    std::cerr << std::endl;
  }
  template <typename... Args>
  static void trace_fail(const TokenIterator &begin, const TokenIterator &end,
                         Args... args) {
    if (!trace_enabled())
      return;
    indent_level(-1);
    std::cerr << "< " << ParserContext::name << " [FAIL]";
    output_tokens(begin, end);
    std::cerr << std::endl;
  }
};

using ParseTraits = support::ParseStateTraits<TokenIterator, NodeVariant>;

} // namespace networkprotocoldsl::parser::grammar

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_TRAITS_HPP