#ifndef INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_GENERATE_RUNNER_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_GENERATE_RUNNER_HPP

#include <networkprotocoldsl/codegen/outputcontext.hpp>
#include <networkprotocoldsl/codegen/protocolinfo.hpp>

#include <string>
#include <vector>

namespace networkprotocoldsl::codegen {

/**
 * Result of generating the runner component.
 *
 * The runner provides a callback-based interface using C++20 concepts,
 * wrapping the lower-level state machine with a higher-level API.
 *
 * Generated types:
 *   - ServerHandlerConcept - Concept defining required handler methods
 *   - ClientHandlerConcept - Concept defining required handler methods
 *   - ServerRunner<Handler> - Template runner for server side
 *   - ClientRunner<Handler> - Template runner for client side
 *
 * Handlers implement on_<StateName>() methods with overloads for each
 * message type that can arrive at that state. The runner uses std::visit
 * to dispatch to the correct overload based on the parsed message type.
 */
struct RunnerResult {
  std::string header;
  std::string source;
  std::vector<std::string> errors;
};

/**
 * Generate the runner component.
 *
 * This generates a concept-based wrapper around the state machine.
 * Handlers must satisfy the generated concept by implementing
 * on_<StateName>() methods for each state where messages are received.
 *
 * Example usage:
 *   struct MyHandler {
 *       StateOutput on_StateName(const MessageData& msg) const;
 *   };
 *   ServerRunner<MyHandler> runner(handler);
 *   runner.on_bytes_received(data);
 *   // Handler methods are invoked when messages are parsed
 *
 * @param ctx The output context (namespace, etc.)
 * @param info The protocol information
 * @return The generated header and source code
 */
RunnerResult generate_runner(const OutputContext &ctx,
                             const ProtocolInfo &info);

} // namespace networkprotocoldsl::codegen

#endif // INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_GENERATE_RUNNER_HPP
