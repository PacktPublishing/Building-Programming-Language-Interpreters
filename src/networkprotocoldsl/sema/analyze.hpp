#ifndef INCLUDED_NETWORKPROTOCOLDSL_SEMA_ANALYZE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_SEMA_ANALYZE_HPP

#include <memory>
#include <optional>

#include <networkprotocoldsl/parser/tree/protocoldescription.hpp>
#include <networkprotocoldsl/sema/ast/protocol.hpp>

namespace networkprotocoldsl::sema {

std::optional<std::shared_ptr<const ast::Protocol>>
analyze(std::shared_ptr<const parser::tree::ProtocolDescription> &protocol);

}

#endif