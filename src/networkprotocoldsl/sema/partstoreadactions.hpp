#ifndef INCLUDED_NETWORKPROTOCOLDSL_SEMA_PARTSTOREADACTIONS_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_SEMA_PARTSTOREADACTIONS_HPP

#include <memory>
#include <optional>

#include <networkprotocoldsl/parser/tree/messagesequence.hpp>
#include <networkprotocoldsl/sema/ast/action.hpp>

namespace networkprotocoldsl::sema {

std::optional<std::vector<ast::Action>> parts_to_read_actions(
    const std::shared_ptr<const parser::tree::MessageSequence> &parts);

}

#endif // INCLUDED_NETWORKPROTOCOLDSL_SEMA_PARTSTOREADACTIONS_HPP