#ifndef INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_PROTOCOLINFO_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_PROTOCOLINFO_HPP

#include <networkprotocoldsl/parser/tree/messagedata.hpp>
#include <networkprotocoldsl/sema/ast/action.hpp>
#include <networkprotocoldsl/sema/ast/protocol.hpp>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace networkprotocoldsl::codegen {

/**
 * Information about a protocol message for code generation.
 */
struct MessageInfo {
  std::string name;       // Original message name (e.g., "SMTP Server Greeting")
  std::string identifier; // C++ identifier (e.g., "SMTPServerGreeting")
  std::string when_state; // Source state
  std::string then_state; // Target state
  std::string agent;      // "Client" or "Server"
  std::shared_ptr<const parser::tree::MessageData> data;
};

/**
 * Information about a read transition for parser generation.
 */
struct ReadTransitionInfo {
  std::string message_name; // Original message name
  std::string identifier;   // C++ identifier
  std::string when_state;   // Source state
  std::string then_state;   // Target state
  std::string agent;        // "Client" or "Server" - the agent that reads
  std::vector<sema::ast::Action> actions; // Parsing actions
  std::shared_ptr<const parser::tree::MessageData> data;
};

/**
 * Information about a write transition for serializer generation.
 */
struct WriteTransitionInfo {
  std::string message_name; // Original message name
  std::string identifier;   // C++ identifier
  std::string when_state;   // Source state
  std::string then_state;   // Target state
  std::string agent;        // "Client" or "Server" - the agent that writes
  std::vector<sema::ast::Action> actions; // Serialization actions
  std::shared_ptr<const parser::tree::MessageData> data;
};

/**
 * ProtocolInfo extracts and organizes information from a Protocol AST
 * for use by the code generators.
 */
class ProtocolInfo {
public:
  /**
   * Construct a ProtocolInfo from a protocol AST.
   */
  explicit ProtocolInfo(std::shared_ptr<const sema::ast::Protocol> protocol);

  /**
   * Get all unique state names in the protocol.
   */
  const std::set<std::string> &states() const { return states_; }

  /**
   * Get information about all messages in the protocol.
   */
  const std::vector<MessageInfo> &messages() const { return messages_; }

  /**
   * Get information about all read transitions (for parser generation).
   */
  const std::vector<ReadTransitionInfo> &read_transitions() const {
    return read_transitions_;
  }

  /**
   * Get information about all write transitions (for serializer generation).
   */
  const std::vector<WriteTransitionInfo> &write_transitions() const {
    return write_transitions_;
  }

  /**
   * Get the underlying protocol AST.
   */
  const std::shared_ptr<const sema::ast::Protocol> &protocol() const {
    return protocol_;
  }

private:
  std::shared_ptr<const sema::ast::Protocol> protocol_;
  std::set<std::string> states_;
  std::vector<MessageInfo> messages_;
  std::vector<ReadTransitionInfo> read_transitions_;
  std::vector<WriteTransitionInfo> write_transitions_;

  void collect_states();
  void collect_messages();
  void collect_transitions();
};

} // namespace networkprotocoldsl::codegen

#endif // INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_PROTOCOLINFO_HPP
