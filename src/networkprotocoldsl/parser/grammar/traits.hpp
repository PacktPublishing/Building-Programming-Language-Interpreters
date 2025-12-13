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
#include <networkprotocoldsl/parser/tree/tokensequenceoptions.hpp>
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
                 std::shared_ptr<tree::EscapeReplacement>,
                 std::shared_ptr<tree::TokenSequenceOptionValue>,
                 std::shared_ptr<const tree::TokenSequenceOptionPair>,
                 std::shared_ptr<const tree::TokenSequenceOptionsMap>,
                 std::shared_ptr<const tree::Terminator>,
                 std::shared_ptr<const tree::ProtocolDescription>,
                 std::shared_ptr<const tree::MessageSequence>,
                 std::shared_ptr<const tree::MessagePart>,
                 std::shared_ptr<const tree::MessageForLoop>,
                 std::shared_ptr<const tree::MessageDataPair>,
                 std::shared_ptr<const tree::MessageData>,
                 std::shared_ptr<const tree::Message>>;

using TokenIterator = std::vector<lexer::Token>::const_iterator;

// Trait to check for the presence of the stringify method
template <typename T, typename = void>
struct has_stringify : std::false_type {};

template <typename T>
struct has_stringify<T, std::void_t<decltype(std::declval<T>().stringify())>>
    : std::true_type {};

// Placeholder value
constexpr const char *placeholder = "<no stringify>";

static int indent_level(int modify) {
  static int indent = 0;
  indent += modify;
  return indent;
}

static std::string indent_str() {
  std::stringstream r;
  for (int i = 0; i < indent_level(0); ++i) {
    r << ' ';
  }
  r << "(" << indent_level(0) << ")";
  return r.str();
}

static bool trace_enabled() {
  static bool r = getenv("GRAMMAR_TRACER") != nullptr;
  return r;
}

} // namespace

template <typename ParserContext> class Tracer {
public:
  using TokenIterator = ParserContext::TokenIterator;
  static void output_tokens(TokenIterator begin, TokenIterator end) {
    int max = 10;
    std::cerr << " (" << &begin << "-" << &end << ") ";
    std::cerr << " [" << std::distance(begin, end) << "]:";
    TokenIterator b = begin;
    while (max > 0 && b != end) {
      std::cerr << " "
                << std::visit(
                       [&](auto t) {
                         if constexpr (has_stringify<decltype(t)>::value) {
                           return t.stringify();
                         } else {
                           return placeholder;
                         }
                       },
                       *b);
      b++;
      max--;
    }
    if (b != end) {
      std::cerr << "...";
    }
  }

  template <typename... Args>
  static void trace_start(const char *attempt_type, TokenIterator begin,
                          TokenIterator end, Args... args) {
    if (!trace_enabled())
      return;
    std::cerr << indent_str() << "> " << attempt_type << " "
              << ParserContext::name << " (" << sizeof...(args) << "):";
    output_tokens(begin, end);
    std::cerr << std::endl << std::flush;
    indent_level(1);
  }
  template <typename... Args>
  static void trace_success(const char *attempt_type, TokenIterator begin,
                            TokenIterator end, Args... args) {
    if (!trace_enabled())
      return;
    indent_level(-1);
    std::cerr << indent_str() << "< " << attempt_type << " "
              << ParserContext::name << " [SUCCESS]";
    output_tokens(begin, end);
    std::cerr << std::endl << std::flush;
    ;
  }
  template <typename... Args>
  static void trace_fail(const char *attempt_type, TokenIterator begin,
                         TokenIterator end, Args... args) {
    if (!trace_enabled())
      return;
    indent_level(-1);
    std::cerr << indent_str() << "< " << attempt_type << " "
              << ParserContext::name << " [FAIL]";
    output_tokens(begin, end);
    std::cerr << std::endl << std::flush;
  }
};

using ParseTraits = support::ParseStateTraits<TokenIterator, NodeVariant>;

} // namespace networkprotocoldsl::parser::grammar

#endif // INCLUDED_NETWORKPROTOCOLDSL_PARSER_GRAMMAR_TRAITS_HPP