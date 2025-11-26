#ifndef INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_TYPEMAPPING_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_TYPEMAPPING_HPP

#include <networkprotocoldsl/parser/tree/type.hpp>
#include <networkprotocoldsl/parser/tree/typeparametervalue.hpp>

#include <optional>
#include <string>

namespace networkprotocoldsl::codegen {

/**
 * Maps DSL types to their C++ equivalents.
 *
 * Type mapping rules:
 *   int<encoding=AsciiInt, unsigned=True, bits=8>   -> uint8_t
 *   int<encoding=AsciiInt, unsigned=True, bits=16>  -> uint16_t
 *   int<encoding=AsciiInt, unsigned=True, bits=32>  -> uint32_t
 *   int<encoding=AsciiInt, unsigned=False, bits=8>  -> int8_t
 *   int<encoding=AsciiInt, unsigned=False, bits=16> -> int16_t
 *   int<encoding=AsciiInt, unsigned=False, bits=32> -> int32_t
 *   str<...>                                        -> std::string
 *   array<element_type=T, ...>                      -> std::vector<T>
 *   tuple<field1=T1, field2=T2, ...>                -> generated struct
 */

/**
 * Result of type mapping - contains both the C++ type string
 * and any auxiliary definitions needed (e.g., nested struct definitions).
 */
struct TypeMappingResult {
  std::string cpp_type;            // The C++ type to use (e.g., "std::string")
  std::string auxiliary_definitions; // Any struct definitions needed
  bool is_struct = false;          // True if this is a generated struct type
};

/**
 * Convert a DSL type to its C++ equivalent.
 *
 * @param type The DSL type from the parser tree
 * @param struct_name_prefix Prefix for generated struct names (for nested tuples)
 * @return The mapping result, or nullopt if the type is not recognized
 */
std::optional<TypeMappingResult>
type_to_cpp(const std::shared_ptr<const parser::tree::Type> &type,
            const std::string &struct_name_prefix = "");

/**
 * Convert a message name to a valid C++ identifier.
 * E.g., "SMTP Server Greeting" -> "SMTPServerGreeting"
 */
std::string message_name_to_identifier(const std::string &message_name);

/**
 * Convert a state name to a valid C++ enum identifier.
 * E.g., "ClientSendEHLO" -> "ClientSendEHLO"
 */
std::string state_name_to_identifier(const std::string &state_name);

} // namespace networkprotocoldsl::codegen

#endif // INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_TYPEMAPPING_HPP
