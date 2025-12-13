#ifndef NETWORKPROTOCOLDSL_OPERATION_READOCTETSUNTILTERMINATOR_HPP
#define NETWORKPROTOCOLDSL_OPERATION_READOCTETSUNTILTERMINATOR_HPP

#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>

namespace networkprotocoldsl {

namespace operation {

class ReadOctetsUntilTerminator {
  const std::string terminator;
  // Optional escape replacement: when escape_sequence is found in input,
  // it is replaced with escape_char in the captured value
  const std::optional<std::string> escape_char;
  const std::optional<std::string> escape_sequence;

public:
  using Arguments = std::tuple<>;
  ReadOctetsUntilTerminator(const std::string &_t) : terminator(_t) {}
  ReadOctetsUntilTerminator(const std::string &_t,
                            const std::string &_escape_char,
                            const std::string &_escape_sequence)
      : terminator(_t), escape_char(_escape_char),
        escape_sequence(_escape_sequence) {}

  OperationResult operator()(InputOutputOperationContext &ctx,
                             Arguments a) const;
  size_t handle_read(InputOutputOperationContext &ctx,
                     std::string_view in) const;

  std::string_view get_write_buffer(InputOutputOperationContext &ctx) const;
  void handle_eof(InputOutputOperationContext &ctx) const;

  size_t handle_write(InputOutputOperationContext &ctx, size_t s) const;

  // Returns true if the operation has enough data to produce a result.
  bool ready_to_evaluate(InputOutputOperationContext &ctx) const;

  std::string stringify() const {
    std::string result = "ReadOctetsUntilTerminator{terminator: \"" + terminator + "\"";
    if (escape_char.has_value() && escape_sequence.has_value()) {
      result += ", escape_char: \"" + *escape_char + "\", escape_sequence: \"" + *escape_sequence + "\"";
    }
    result += "}";
    return result;
  }
};
static_assert(InputOutputOperationConcept<ReadOctetsUntilTerminator>);

} // namespace operation

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_OPERATION_HPP
