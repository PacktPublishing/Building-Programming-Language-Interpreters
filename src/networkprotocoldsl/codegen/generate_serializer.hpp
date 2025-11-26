#ifndef INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_GENERATE_SERIALIZER_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_GENERATE_SERIALIZER_HPP

#include <networkprotocoldsl/codegen/outputcontext.hpp>
#include <networkprotocoldsl/codegen/protocolinfo.hpp>

#include <string>
#include <utility>
#include <vector>

namespace networkprotocoldsl::codegen {

/**
 * Result of serializer generation.
 */
struct SerializerResult {
  std::string header; // Content of serializer.hpp
  std::string source; // Content of serializer.cpp
  std::vector<std::string> errors;
};

/**
 * Generate the serializer header and source files.
 *
 * Creates sans-IO serializers that:
 * - Accept typed message data structs
 * - Produce output bytes incrementally
 * - Track serialization progress for large messages
 *
 * Generated components:
 * - Individual message serializers (e.g., serialize_SMTPServerGreeting)
 * - Main Serializer class with state-based dispatch
 *
 * @param ctx The output context for namespace and formatting
 * @param info The protocol information
 * @return The generated header and source content, plus any errors
 */
SerializerResult generate_serializer(const OutputContext &ctx,
                                     const ProtocolInfo &info);

} // namespace networkprotocoldsl::codegen

#endif // INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_GENERATE_SERIALIZER_HPP
