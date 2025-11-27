#ifndef INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_GENERATE_PARSER_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_GENERATE_PARSER_HPP

#include <networkprotocoldsl/codegen/outputcontext.hpp>
#include <networkprotocoldsl/codegen/protocolinfo.hpp>

#include <string>
#include <utility>
#include <vector>

namespace networkprotocoldsl::codegen {

/**
 * Result of parser generation.
 */
struct ParserResult {
  std::string header; // Content of parser.hpp
  std::string source; // Content of parser.cpp
  std::vector<std::string> errors;
};

/**
 * Generate the parser header and source files.
 *
 * Creates sans-IO parsers that:
 * - Accept input bytes incrementally
 * - Return parse status (NeedMoreData, Complete, Error)
 * - Track parsing state for resumption
 * - Extract message data into typed structs
 *
 * Generated components:
 * - ParseStatus enum
 * - ParseResult struct
 * - Individual message parsers (e.g., SMTPServerGreetingParser)
 * - Main Parser class with state-based dispatch
 *
 * @param ctx The output context for namespace and formatting
 * @param info The protocol information
 * @return The generated header and source content, plus any errors
 */
ParserResult generate_parser(const OutputContext &ctx,
                             const ProtocolInfo &info);

} // namespace networkprotocoldsl::codegen

#endif // INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_GENERATE_PARSER_HPP
