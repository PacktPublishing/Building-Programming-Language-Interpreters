#include <networkprotocoldsl/parser/tree/protocoldescription.hpp>
#include <networkprotocoldsl/sema/analyze.hpp>
#include <networkprotocoldsl/sema/ast/protocol.hpp>

namespace networkprotocoldsl::sema {

std::optional<std::shared_ptr<const ast::Protocol>>
analyze(std::shared_ptr<const parser::tree::ProtocolDescription> &protocol) {
  return std::nullopt;
}

} // namespace networkprotocoldsl::sema