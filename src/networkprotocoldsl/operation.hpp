#ifndef NETWORKPROTOCOLDSL_OPERATION_HPP
#define NETWORKPROTOCOLDSL_OPERATION_HPP

#include <variant>

#include <networkprotocoldsl/operation/add.hpp>
#include <networkprotocoldsl/operation/dynamiclist.hpp>
#include <networkprotocoldsl/operation/eq.hpp>
#include <networkprotocoldsl/operation/functioncall.hpp>
#include <networkprotocoldsl/operation/if.hpp>
#include <networkprotocoldsl/operation/int32literal.hpp>
#include <networkprotocoldsl/operation/inttoascii.hpp>
#include <networkprotocoldsl/operation/lesserequal.hpp>
#include <networkprotocoldsl/operation/lexicalpadget.hpp>
#include <networkprotocoldsl/operation/lexicalpadinitialize.hpp>
#include <networkprotocoldsl/operation/lexicalpadinitializeglobal.hpp>
#include <networkprotocoldsl/operation/lexicalpadset.hpp>
#include <networkprotocoldsl/operation/multiply.hpp>
#include <networkprotocoldsl/operation/opsequence.hpp>
#include <networkprotocoldsl/operation/readint32native.hpp>
#include <networkprotocoldsl/operation/readintfromascii.hpp>
#include <networkprotocoldsl/operation/readoctetsuntilterminator.hpp>
#include <networkprotocoldsl/operation/readstaticoctets.hpp>
#include <networkprotocoldsl/operation/staticcallable.hpp>
#include <networkprotocoldsl/operation/subtract.hpp>
#include <networkprotocoldsl/operation/unarycallback.hpp>
#include <networkprotocoldsl/operation/writeint32native.hpp>
#include <networkprotocoldsl/operation/writeoctets.hpp>
#include <networkprotocoldsl/operation/writestaticoctets.hpp>

namespace networkprotocoldsl {

/**
 * Operation is a variant of all known operation types.
 *
 * This allows us to implement a type-safe dispatch on all the
 * operations that may happen in the execution of the interpreted
 * code.
 */
using Operation = std::variant<
    operation::Add, operation::DynamicList, operation::Eq,
    operation::FunctionCall, operation::If, operation::Int32Literal,
    operation::IntToAscii, operation::LesserEqual, operation::LexicalPadGet,
    operation::LexicalPadInitialize, operation::LexicalPadInitializeGlobal,
    operation::LexicalPadSet, operation::Multiply, operation::OpSequence,
    operation::ReadInt32Native, operation::ReadIntFromAscii,
    operation::ReadOctetsUntilTerminator, operation::ReadStaticOctets,
    operation::StaticCallable, operation::Subtract, operation::UnaryCallback,
    operation::WriteInt32Native, operation::WriteOctets,
    operation::WriteStaticOctets>;

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_OPERATION_HPP
