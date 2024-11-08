#ifndef INCLUDED_NETWORKPROTOCOLDSL_PARSER_SUPPORT_RECURSIVEPARSER_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_PARSER_SUPPORT_RECURSIVEPARSER_HPP

#include <any>
#include <iostream>
#include <optional>
#include <type_traits>
#include <variant>
#include <vector>

namespace networkprotocoldsl::parser::support {

template <typename TOKENITERATOR, typename NODEVARIANT>
struct ParseStateTraits {
  using TokenIterator = TOKENITERATOR;
  using ParseNode = NODEVARIANT;
  struct ParseStateReturn {
    std::optional<ParseNode> node;
    TokenIterator begin;
    TokenIterator end;
  };
};

template <typename ParseStateTraits, typename T, typename... Args>
concept has_conditional_partial_match = requires(Args... args) {
  { T::contitional_partial_match(args...) } -> std::convertible_to<bool>;
};

template <typename ParseStateTraits, typename T, typename... Args>
struct is_conditional_partial_match {
  static constexpr bool value =
      has_conditional_partial_match<ParseStateTraits, T, Args...>;
};

template <typename ParseStateTraits, typename T, typename... Args>
concept has_partial_match = requires(Args... args) {
  {T::partial_match(args...)};
};

template <typename ParseStateTraits, typename T, typename... Args>
struct is_partial_match {
  static constexpr bool value = has_partial_match<ParseStateTraits, T, Args...>;
};

template <typename ParseStateTraits, typename T, typename... Args>
concept has_maybe_match = requires(Args... args) {
  {T::maybe_match(args...)};
};

template <typename ParseStateTraits, typename T, typename... Args>
struct is_maybe_match {
  static constexpr bool value = has_maybe_match<ParseStateTraits, T, Args...>;
};

template <typename ParseStateTraits, typename T, typename... Args>
concept has_match = requires(typename ParseStateTraits::TokenIterator begin,
                             typename ParseStateTraits::TokenIterator end,
                             Args... args) {
  {T::match(begin, end, args...)};
};

template <typename ParseStateTraits, typename T, typename... Args>
struct is_match {
  static constexpr bool value = has_match<ParseStateTraits, T, Args...>;
};

template <typename ParseStateTraits, typename T, typename... Args>
concept has_recurse_one = requires(Args... args) {
  {T::recurse_one(args...)};
};

template <typename ParseStateTraits, typename T, typename... Args>
struct is_recurse_one {
  static constexpr bool value = has_recurse_one<ParseStateTraits, T, Args...>;
};

template <typename ParseStateTraits, typename T, typename... Args>
struct recurse_one {
  using type = std::remove_pointer_t<decltype(std::declval<T>().recurse_one(
      std::declval<Args>()...))>;
};

template <typename ParseStateTraits, typename T, typename... Args>
concept has_recurse_maybe = requires(Args... args) {
  {T::recurse_maybe(args...)};
};

template <typename ParseStateTraits, typename T, typename... Args>
struct is_recurse_maybe {
  static constexpr bool value = has_recurse_maybe<ParseStateTraits, T, Args...>;
};

template <typename ParseStateTraits, typename T, typename... Args>
struct recurse_maybe {
  using type = std::remove_pointer_t<decltype(std::declval<T>().recurse_maybe(
      std::declval<Args>()...))>;
};

template <typename ParseStateTraits, typename T, typename... Args>
concept has_recurse_any = requires(Args... args) {
  {T::recurse_any(args...)};
};

template <typename ParseStateTraits, typename T, typename... Args>
struct is_recurse_any {
  static constexpr bool value = has_recurse_any<ParseStateTraits, T, Args...>;
};

template <typename ParseStateTraits, typename T, typename... Args>
struct recurse_any {
  using Tuple = std::remove_pointer_t<decltype(std::declval<T>().recurse_any(
      std::declval<Args>()...))>;

  template <typename H, typename... Ts>
  static ParseStateTraits::ParseStateReturn
  try_parse_any(typename ParseStateTraits::TokenIterator begin,
                typename ParseStateTraits::TokenIterator end) {
    if constexpr (sizeof...(Ts) > 0) {
      typename ParseStateTraits::ParseStateReturn result = H::parse(begin, end);
      if (result.node.has_value()) {
        return result;
      } else {
        return try_parse_any<Ts...>(begin, end);
      }
    } else {
      return H::parse(begin, end);
    }
  }

  template <std::size_t... I>
  static typename ParseStateTraits::ParseStateReturn
  parse_any_to_indices(typename ParseStateTraits::TokenIterator begin,
                       typename ParseStateTraits::TokenIterator end,
                       std::index_sequence<I...>) {
    return try_parse_any<std::tuple_element_t<I, Tuple>...>(begin, end);
  }

  static typename ParseStateTraits::ParseStateReturn
  parse_any(typename ParseStateTraits::TokenIterator begin,
            typename ParseStateTraits::TokenIterator end) {
    return parse_any_to_indices(
        begin, end, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
  }
};

template <typename ParseStateTraits, typename T, typename... Args>
concept has_recurse_many = requires(Args... args) {
  {T::recurse_many(args...)};
};

template <typename ParseStateTraits, typename T, typename... Args>
struct is_recurse_many {
  static constexpr bool value = has_recurse_many<ParseStateTraits, T, Args...>;
};

template <typename ParseStateTraits, typename T, typename... Args>
struct recurse_many {
  using type = std::remove_pointer_t<decltype(std::declval<T>().recurse_many(
      std::declval<Args>()...))>;
};

template <typename ParseStateTraits, typename T, typename... Args>
concept has_recurse_many_has_separator = requires(Args... args) {
  {T::recurse_many_has_separator(args...)};
};

template <typename ParseStateTraits, typename T, typename... Args>
struct is_recurse_many_has_separator {
  static constexpr bool value =
      has_recurse_many_has_separator<ParseStateTraits, T, Args...>;
};

template <typename ParseStateTraits, typename T, typename... Args>
concept has_recurse_many_separator = requires(Args... args) {
  {T::recurse_many_separator(args...)};
};

template <typename ParseStateTraits, typename T, typename... Args>
struct is_recurse_many_separator {
  static constexpr bool value =
      has_recurse_many_separator<ParseStateTraits, T, Args...>;
};

// Helper to check if all elements in a vector of variants hold the same type
template <typename Variant>
std::optional<std::size_t>
get_common_variant_index(const std::vector<Variant> &variants) {
  if (variants.empty()) {
    return std::nullopt;
  }
  std::size_t common_index = variants[0].index();
  for (const auto &var : variants) {
    if (var.index() != common_index) {
      return std::nullopt;
    }
  }
  return common_index;
}

template <class ParserContext, typename ParseStateTraits>
class RecursiveParser {
public:
  using ParseStateReturn = typename ParseStateTraits::ParseStateReturn;
  using TokenIterator = typename ParseStateTraits::TokenIterator;
  using ParseNode = typename ParseStateTraits::ParseNode;

  template <typename... Args>
  static ParseStateReturn parse(TokenIterator begin, TokenIterator end,
                                Args... args) {
    if constexpr (is_partial_match<ParseStateTraits, ParserContext,
                                   Args...>::value) {
      if (begin == end)
        return {std::nullopt, begin, end};
      auto token = *begin++;
      return std::visit(
          [=](auto t) { return ParserContext::parse(begin, end, args..., t); },
          token);
    } else if constexpr (is_conditional_partial_match<
                             ParseStateTraits, ParserContext, Args...>::value) {
      if (begin == end)
        return {std::nullopt, begin, end};
      if (ParserContext::conditional_partial_match(args...)) {
        auto token = *begin++;
        return std::visit(
            [=](auto t) {
              return ParserContext::parse(begin, end, args..., t);
            },
            token);
      } else {
        return {std::nullopt, begin, end};
      }
    } else if constexpr (is_recurse_one<ParseStateTraits, ParserContext,
                                        Args...>::value) {
      ParseStateReturn r =
          recurse_one<ParseStateTraits, ParserContext, Args...>::type::parse(
              begin, end);
      if (!r.node.has_value())
        return r;
      return std::visit(
          [=](auto t) {
            return ParserContext::parse(r.begin, end, args..., t);
          },
          r.node.value());
    } else if constexpr (is_recurse_maybe<ParseStateTraits, ParserContext,
                                          Args...>::value) {
      ParseStateReturn r =
          recurse_maybe<ParseStateTraits, ParserContext, Args...>::type::parse(
              begin, end);
      if (!r.node.has_value()) {
        return ParserContext::parse(begin, end, args..., std::nullopt);
      } else {
        return std::visit(
            [=](auto t) {
              return ParserContext::parse(r.begin, end, args..., t);
            },
            r.node.value());
      }
    } else if constexpr (is_recurse_any<ParseStateTraits, ParserContext,
                                        Args...>::value) {
      ParseStateReturn r =
          recurse_any<ParseStateTraits, ParserContext, Args...>::parse_any(
              begin, end);
      if (!r.node.has_value()) {
        return r;
      } else {
        return std::visit(
            [=](auto t) {
              return ParserContext::parse(r.begin, end, args..., t);
            },
            r.node.value());
      }
    } else if constexpr (is_recurse_many<ParseStateTraits, ParserContext,
                                         Args...>::value) {
      std::vector<ParseNode> nodes;
      while (true) {
        ParseStateReturn r =
            recurse_many<ParseStateTraits, ParserContext, Args...>::type::parse(
                begin, end);
        if (!r.node.has_value()) {
          break;
        } else {
          begin = r.begin;
          nodes.push_back(r.node.value());
          if constexpr (is_recurse_many_has_separator<
                            ParseStateTraits, ParserContext, Args...>::value) {
            bool found_separator = false;
            auto attempt = begin;
            auto t = *attempt++;
            std::visit(
                [&](auto &&t) {
                  if constexpr (is_recurse_many_separator<
                                    ParseStateTraits, ParserContext, Args...,
                                    decltype(t)>::value) {
                    found_separator = true;
                  }
                },
                t);
            if (!found_separator) {
              break;
            } else {
              begin = attempt;
            }
          }
        }
      }
      if (!nodes.empty()) {
        auto common_index = get_common_variant_index<ParseNode>(nodes);
        if (common_index.has_value()) {
          return std::visit(
              [&](auto arg) {
                std::vector<decltype(arg)> output;
                for (auto v : nodes) {
                  output.push_back(std::get<decltype(arg)>(v));
                }
                return ParserContext::parse(begin, end, args..., output);
              },
              nodes.at(0));
        }
      }
      return ParserContext::parse(begin, end, args..., nodes);
    } else if constexpr (is_match<ParseStateTraits, ParserContext,
                                  Args...>::value) {
      return ParserContext::match(begin, end, args...);
    } else {
      return {std::nullopt, begin, end};
    }
  }
};

} // namespace networkprotocoldsl::parser::support

#endif