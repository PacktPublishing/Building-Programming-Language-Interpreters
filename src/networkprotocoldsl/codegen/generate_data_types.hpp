#ifndef INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_GENERATE_DATA_TYPES_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_GENERATE_DATA_TYPES_HPP

#include <networkprotocoldsl/codegen/outputcontext.hpp>
#include <networkprotocoldsl/codegen/protocolinfo.hpp>

#include <string>
#include <utility>
#include <vector>

namespace networkprotocoldsl::codegen {

/**
 * Result of data types generation.
 */
struct DataTypesResult {
  std::string header; // Content of data_types.hpp
  std::string source; // Content of data_types.cpp
  std::vector<std::string> errors;
};

/**
 * Generate the data types header and source files.
 *
 * Creates struct definitions for each message's data, mapping DSL types
 * to C++ types. Each message gets a corresponding Data struct
 * (e.g., SMTPServerGreetingData).
 *
 * @param ctx The output context for namespace and formatting
 * @param info The protocol information
 * @return The generated header and source content, plus any errors
 */
DataTypesResult generate_data_types(const OutputContext &ctx,
                                    const ProtocolInfo &info);

} // namespace networkprotocoldsl::codegen

#endif // INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_GENERATE_DATA_TYPES_HPP
