#include "transitionlookahead.hpp"

namespace networkprotocoldsl {
namespace operation {

OperationResult
TransitionLookahead::operator()(InputOutputOperationContext &ctx,
                                const Arguments &) const {
  bool all_permanently_invalid = true;
  for (const auto &condition : conditions) {
    const auto &cond = condition.first;
    const auto &target_state = condition.second;

    auto [is_valid, is_permanently_invalid] = std::visit(
        [&](const auto &c) { return match_condition(ctx, c); }, cond);
    if (is_valid) {
      return value::Octets{std::make_shared<std::string>(target_state)};
    }
    if (!is_permanently_invalid) {
      all_permanently_invalid = false;
    }
  }
  if (all_permanently_invalid) {
    return value::RuntimeError::ProtocolMismatchError;
  }
  return ReasonForBlockedOperation::WaitingForRead;
}

std::size_t TransitionLookahead::handle_read(InputOutputOperationContext &ctx,
                                             std::string_view sv) const {
  ctx.buffer = sv;
  return 0;
}

std::string_view
TransitionLookahead::get_write_buffer(InputOutputOperationContext &ctx) const {
  return {};
}

std::size_t TransitionLookahead::handle_write(InputOutputOperationContext &ctx,
                                              std::size_t s) const {
  return 0;
}

void TransitionLookahead::handle_eof(InputOutputOperationContext &ctx) const {
  ctx.eof = true;
}

std::string TransitionLookahead::stringify() const {
  std::ostringstream oss;
  oss << "TransitionLookahead(";
  for (const auto &condition : conditions) {
    std::visit([&](const auto &c) { oss << condition_to_string(c); },
               condition.first);
    oss << " -> " << condition.second << ", ";
  }
  std::string result = oss.str();
  if (!conditions.empty()) {
    result.pop_back();
    result.pop_back();
  }
  result += ")";
  return result;
}

std::pair<bool, bool>
TransitionLookahead::match_condition(InputOutputOperationContext &ctx,
                                     const EOFCondition &) {
  if (ctx.eof) {
    return {true, false};
  } else {
    if (ctx.buffer.empty()) {
      return {false, false};
    } else {
      return {false, true};
    }
  }
}

std::pair<bool, bool>
TransitionLookahead::match_condition(InputOutputOperationContext &ctx,
                                     const MatchUntilTerminator &c) {
  auto it = ctx.buffer.find(c.terminator);
  if (it != std::string::npos) {
    return {true, false};
  } else {
    return {false, ctx.eof};
  }
}

std::pair<bool, bool>
TransitionLookahead::match_condition(InputOutputOperationContext &ctx,
                                     const std::string &c) {
  if (ctx.buffer.size() <= c.size()) {
    if (ctx.buffer.compare(0, ctx.buffer.size(),
                           c.substr(0, ctx.buffer.size())) != 0) {
      return {false, true};
    } else {
      return {false, ctx.eof};
    }
  } else {
    if (ctx.buffer.compare(0, c.size(), c) == 0) {
      return {true, false};
    } else {
      return {false, true};
    }
  }
}

std::string TransitionLookahead::condition_to_string(const EOFCondition &) {
  return "EOF";
}

std::string
TransitionLookahead::condition_to_string(const MatchUntilTerminator &c) {
  return "MatchUntilTerminator(" + c.terminator + ")";
}

std::string TransitionLookahead::condition_to_string(const std::string &c) {
  return "StaticString(" + c + ")";
}

bool TransitionLookahead::ready_to_evaluate(
    InputOutputOperationContext &ctx) const {
  // The operation is ready when we can definitively decide:
  // either a condition matches, or all conditions are permanently invalid
  bool all_permanently_invalid = true;
  for (const auto &condition : conditions) {
    const auto &cond = condition.first;

    auto [is_valid, is_permanently_invalid] = std::visit(
        [&](const auto &c) { return match_condition(ctx, c); }, cond);
    if (is_valid) {
      return true; // Found a match, we're ready
    }
    if (!is_permanently_invalid) {
      all_permanently_invalid = false;
    }
  }
  // Ready if all conditions are permanently invalid (will return error)
  return all_permanently_invalid;
}

} // namespace operation
} // namespace networkprotocoldsl
