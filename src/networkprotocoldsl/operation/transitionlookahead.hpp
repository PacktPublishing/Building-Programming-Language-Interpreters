#ifndef NETWORKPROTOCOLDSL_OPERATION_TRANSITIONLOOKAHEAD_HPP
#define NETWORKPROTOCOLDSL_OPERATION_TRANSITIONLOOKAHEAD_HPP

#include <networkprotocoldsl/operationconcepts.hpp>
#include <networkprotocoldsl/value.hpp>

#include <sstream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace networkprotocoldsl {
namespace operation {

struct TransitionLookahead {
  struct EOFCondition {};

  struct MatchUntilTerminator {
    std::string terminator;
  };

  using TransitionCondition =
      std::variant<EOFCondition, MatchUntilTerminator, std::string>;

  std::vector<std::pair<TransitionCondition, std::string>>
      conditions; // pair of condition and target state

  using Arguments = std::tuple<>;

  OperationResult operator()(InputOutputOperationContext &ctx,
                             const Arguments &) const;
  std::size_t handle_read(InputOutputOperationContext &ctx,
                          std::string_view sv) const;
  std::string_view get_write_buffer(InputOutputOperationContext &ctx) const;
  std::size_t handle_write(InputOutputOperationContext &ctx,
                           std::size_t s) const;
  void handle_eof(InputOutputOperationContext &ctx) const;
  std::string stringify() const;

private:
  static std::pair<bool, bool> match_condition(InputOutputOperationContext &ctx,
                                               const EOFCondition &);
  static std::pair<bool, bool> match_condition(InputOutputOperationContext &ctx,
                                               const MatchUntilTerminator &c);
  static std::pair<bool, bool> match_condition(InputOutputOperationContext &ctx,
                                               const std::string &c);
  static std::string condition_to_string(const EOFCondition &);
  static std::string condition_to_string(const MatchUntilTerminator &c);
  static std::string condition_to_string(const std::string &c);
};

} // namespace operation
} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_OPERATION_TRANSITIONLOOKAHEAD_HPP
