#include <networkprotocoldsl/sema/partstowriteactions.hpp>

namespace networkprotocoldsl::sema {

std::optional<std::vector<ast::Action>> parts_to_write_actions(
    const std::shared_ptr<const parser::tree::MessageSequence> &parts) {
  return std::nullopt;
}

} // namespace networkprotocoldsl::sema
