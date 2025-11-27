#ifndef INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_GENERATE_STATES_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_GENERATE_STATES_HPP

#include <networkprotocoldsl/codegen/outputcontext.hpp>
#include <networkprotocoldsl/codegen/protocolinfo.hpp>

#include <string>
#include <utility>
#include <vector>

namespace networkprotocoldsl::codegen {

/**
 * Result of states generation.
 */
struct StatesResult {
  std::string header; // Content of states.hpp
  std::string source; // Content of states.cpp
  std::vector<std::string> errors;
};

/**
 * Generate the states header and source files.
 *
 * Creates:
 * - State enum with all protocol states
 * - Input variant types for each state (possible incoming messages)
 * - Output variant types for each state (possible outgoing messages)
 *
 * @param ctx The output context for namespace and formatting
 * @param info The protocol information
 * @return The generated header and source content, plus any errors
 */
StatesResult generate_states(const OutputContext &ctx,
                             const ProtocolInfo &info);

} // namespace networkprotocoldsl::codegen

#endif // INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_GENERATE_STATES_HPP
