#ifndef NETWORKPROTOCOLDSL_GENERATE_HPP
#define NETWORKPROTOCOLDSL_GENERATE_HPP

#include <networkprotocoldsl/optree.hpp>
#include <networkprotocoldsl/sema/ast/protocol.hpp>

#include <memory>
#include <optional>

namespace networkprotocoldsl {
namespace generate {

std::optional<std::shared_ptr<const OpTree>>
client(const std::shared_ptr<const sema::ast::Protocol> &protocol);
std::optional<std::shared_ptr<const OpTree>>
server(const std::shared_ptr<const sema::ast::Protocol> &protocol);

} // namespace generate
} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_GENERATE_HPP