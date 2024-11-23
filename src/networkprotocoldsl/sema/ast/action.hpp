#ifndef INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_ACTION_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_SEMA_AST_ACTION_HPP

#include <memory>
#include <variant>

#include <networkprotocoldsl/sema/ast/action/read.hpp>
#include <networkprotocoldsl/sema/ast/action/write.hpp>

namespace networkprotocoldsl::sema::ast {

namespace action {
struct Loop; // this is recursive. we need to forward declare it here because
             // Action is a template alias and that can't be forward declared.
}

using Action =
    std::variant<action::ReadStaticOctets, action::ReadOctetsUntilTerminator,
                 action::WriteFromIdentifier, action::WriteStaticOctets,
                 action::Loop>;

} // namespace networkprotocoldsl::sema::ast

// Include the rest of the action headers here because they are needed for the
// definition of the Action variant.
#include <networkprotocoldsl/sema/ast/action/loop.hpp>

#endif