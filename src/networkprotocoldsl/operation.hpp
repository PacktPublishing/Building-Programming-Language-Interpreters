#ifndef NETWORKPROTOCOLDSL_OPERATION_HPP
#define NETWORKPROTOCOLDSL_OPERATION_HPP

#include <variant>

#include <networkprotocoldsl/operation/add.hpp>
#include <networkprotocoldsl/operation/eq.hpp>
#include <networkprotocoldsl/operation/if.hpp>
#include <networkprotocoldsl/operation/int32literal.hpp>
#include <networkprotocoldsl/operation/readint32native.hpp>
#include <networkprotocoldsl/operation/staticcallable.hpp>
#include <networkprotocoldsl/operation/unarycallback.hpp>
#include <networkprotocoldsl/operation/writeint32native.hpp>

namespace networkprotocoldsl {

/**
 * Operation is a variant of all known operation types.
 *
 * This allows us to implement a type-safe dispatch on all the
 * operations that may happen in the execution of the interpreted
 * code.
 */
using Operation =
    std::variant<operation::Int32Literal, operation::Add, operation::Eq,
                 operation::If, operation::UnaryCallback,
                 operation::ReadInt32Native, operation::StaticCallable,
                 operation::WriteInt32Native>;

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_OPERATION_HPP
