#ifndef INCLUDED_NETWORKPROTOCOLDSL_OPERATION_WRITEOCTETSWITHESCAPE_H
#define INCLUDED_NETWORKPROTOCOLDSL_OPERATION_WRITEOCTETSWITHESCAPE_H

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

// WriteOctetsWithEscape writes octets, replacing escape_char with escape_sequence
// For example, if escape_char="\n" and escape_sequence="\r\n ",
// writing "hello\nworld" produces "hello\r\n world"
class WriteOctetsWithEscape {
  const std::string escape_char;
  const std::string escape_sequence;

public:
  using Arguments = std::tuple<Value>;
  WriteOctetsWithEscape(const std::string &_escape_char,
                        const std::string &_escape_sequence)
      : escape_char(_escape_char), escape_sequence(_escape_sequence) {}

  OperationResult operator()(InputOutputOperationContext &ctx,
                             Arguments a) const;
  size_t handle_read(InputOutputOperationContext &ctx,
                     std::string_view in) const;

  std::string_view get_write_buffer(InputOutputOperationContext &ctx) const;
  void handle_eof(InputOutputOperationContext &ctx) const;

  size_t handle_write(InputOutputOperationContext &ctx, size_t s) const;

  bool ready_to_evaluate(InputOutputOperationContext &ctx) const {
    return true; // Write operations don't wait for input
  }

  std::string stringify() const {
    return "WriteOctetsWithEscape{escape_char: \"" + escape_char +
           "\", escape_sequence: \"" + escape_sequence + "\"}";
  }
};
static_assert(InputOutputOperationConcept<WriteOctetsWithEscape>);

} // namespace operation

} // namespace networkprotocoldsl

#endif
