#ifndef INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_GENERATE_STATE_MACHINE_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_GENERATE_STATE_MACHINE_HPP

#include <networkprotocoldsl/codegen/outputcontext.hpp>
#include <networkprotocoldsl/codegen/protocolinfo.hpp>

#include <string>
#include <utility>
#include <vector>

namespace networkprotocoldsl::codegen {

/**
 * Result of state machine generation.
 */
struct StateMachineResult {
  std::string header; // Content of state_machine.hpp
  std::string source; // Content of state_machine.cpp
  std::vector<std::string> errors;
};

/**
 * Generate the state machine header and source files.
 *
 * Creates state machine coordinators that:
 * - Coordinate between IO, parser, and callback threads
 * - Manage state transitions
 * - Provide thread-safe queues for data passing
 * - Support the sans-IO architecture
 *
 * Generated components:
 * - ClientStateMachine - State machine from client perspective
 * - ServerStateMachine - State machine from server perspective
 *
 * @param ctx The output context for namespace and formatting
 * @param info The protocol information
 * @return The generated header and source content, plus any errors
 */
StateMachineResult generate_state_machine(const OutputContext &ctx,
                                          const ProtocolInfo &info);

} // namespace networkprotocoldsl::codegen

#endif // INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_GENERATE_STATE_MACHINE_HPP
