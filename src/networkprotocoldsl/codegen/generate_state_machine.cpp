#include <networkprotocoldsl/codegen/generate_state_machine.hpp>
#include <networkprotocoldsl/codegen/typemapping.hpp>

#include <sstream>

namespace networkprotocoldsl::codegen {

namespace {

// Filter read transitions for a specific agent
// Client reads what Server writes, Server reads what Client writes
std::vector<const ReadTransitionInfo *>
get_read_transitions_for_agent(const ProtocolInfo &info, bool is_client) {
  std::vector<const ReadTransitionInfo *> result;
  // Client reads messages written by Server, Server reads messages written by Client
  // Read transitions are stored by the agent that reads them
  std::string agent = is_client ? "Client" : "Server";
  for (const auto &rt : info.read_transitions()) {
    if (rt.agent == agent) {
      result.push_back(&rt);
    }
  }
  return result;
}

// Filter write transitions for a specific agent
std::vector<const WriteTransitionInfo *>
get_write_transitions_for_agent(const ProtocolInfo &info, bool is_client) {
  std::vector<const WriteTransitionInfo *> result;
  std::string agent = is_client ? "Client" : "Server";
  for (const auto &wt : info.write_transitions()) {
    if (wt.agent == agent) {
      result.push_back(&wt);
    }
  }
  return result;
}

// Collect states that have incoming messages (for take_message methods), filtered by agent
std::map<std::string, std::vector<const ReadTransitionInfo *>>
get_messages_by_then_state(const ProtocolInfo &info, bool is_client) {
  std::map<std::string, std::vector<const ReadTransitionInfo *>> result;
  auto read_trans = get_read_transitions_for_agent(info, is_client);
  for (const auto *rt : read_trans) {
    result[rt->then_state].push_back(rt);
  }
  return result;
}

// Generate the state machine class header
void generate_state_machine_class_header(std::ostringstream &header,
                                         const std::string &class_name,
                                         const ProtocolInfo &info,
                                         bool is_client) {
  const auto &states = info.states();
  
  // Get agent-specific transitions
  auto read_trans = get_read_transitions_for_agent(info, is_client);
  auto write_trans = get_write_transitions_for_agent(info, is_client);

  // Group read transitions by their target state (then_state)
  auto messages_by_then_state = get_messages_by_then_state(info, is_client);

  header << "/**\n";
  header << " * " << class_name << " - Sans-IO state machine for "
         << (is_client ? "client" : "server") << " side.\n";
  header << " *\n";
  header << " * Usage:\n";
  header << " * 1. Call on_bytes_received() when data arrives from the network\n";
  header << " * 2. Check has_message() to see if a complete message was parsed\n";
  header << " * 3. Use message_state() to know which state's message is available\n";
  header << " * 4. Call the appropriate take_*_message() for that state\n";
  header << " * 5. Check has_pending_output() and use pending_output()/bytes_written()\n";
  header << " * 6. Call send_*() methods to queue outgoing messages\n";
  header << " */\n";
  header << "class " << class_name << " {\n";
  header << "public:\n";
  header << "    " << class_name << "();\n";
  header << "    \n";
  header << "    // Current protocol state\n";
  header << "    State current_state() const { return current_state_; }\n";
  header << "    \n";
  header << "    // Process received bytes, returns number of bytes consumed\n";
  header << "    size_t on_bytes_received(std::string_view data);\n";
  header << "    \n";
  header << "    // Check if there's a complete message available\n";
  header << "    bool has_message() const { return has_message_; }\n";
  header << "    \n";
  header << "    // Get which state the pending message transitioned TO\n";
  header << "    // (use this to determine which take_*_message() to call)\n";
  header << "    State message_state() const { return message_state_; }\n";
  header << "    \n";

  // Generate state-specific take_message methods
  header << "    // State-specific message accessors\n";
  header << "    // Call the one matching message_state() after has_message() returns true\n";
  for (const auto &[state_name, transitions] : messages_by_then_state) {
    if (transitions.empty())
      continue;
    std::string state_id = state_name_to_identifier(state_name);
    header << "    " << state_id << "Input take_" << state_id << "_message();\n";
  }

  header << "    \n";
  header << "    // Check if there's output data pending\n";
  header << "    bool has_pending_output() const { return !output_buffer_.empty(); }\n";
  header << "    \n";
  header << "    // Get pending output bytes (does not consume them)\n";
  header << "    std::string_view pending_output() const { return output_buffer_; }\n";
  header << "    \n";
  header << "    // Mark bytes as written (consume from output buffer)\n";
  header << "    void bytes_written(size_t count);\n";
  header << "    \n";
  header << "    // Check if connection is closed\n";
  header << "    bool is_closed() const { return current_state_ == State::Closed; }\n";
  header << "    \n";
  header << "    // Signal end-of-file on input stream\n";
  header << "    // Call this when the connection is closed by the remote peer.\n";
  header << "    // This notifies parsers that no more data will arrive, allowing them to\n";
  header << "    // complete parsing if they were waiting for a terminator at EOF.\n";
  header << "    void on_eof();\n";
  header << "    \n";

  // Generate send methods for write transitions
  header << "    // Methods to send outgoing messages\n";
  for (const auto *wt : write_trans) {
    header << "    void send_" << wt->identifier << "(const " << wt->identifier
           << "Data& data);\n";
    header << "    void send_" << wt->identifier << "(" << wt->identifier
           << "Data&& data);\n";
  }

  header << "\n";
  header << "private:\n";
  // Initial state should be "Open" - the standard protocol entry point
  if (states.count("Open") > 0) {
    header << "    State current_state_ = State::Open;\n";
  } else if (!states.empty()) {
    header << "    State current_state_ = State::"
           << state_name_to_identifier(*states.begin()) << ";\n";
  } else {
    header << "    State current_state_{};\n";
  }
  header << "    bool has_message_ = false;\n";
  header << "    State message_state_{}; // Which state the message transitioned to\n";
  header << "    std::string output_buffer_;\n";
  header << "    \n";

  // Add storage for each state's pending message using std::optional
  header << "    // Pending message storage (one per target state)\n";
  for (const auto &[state_name, transitions] : messages_by_then_state) {
    if (transitions.empty())
      continue;
    std::string state_id = state_name_to_identifier(state_name);
    header << "    std::optional<" << state_id << "Input> pending_" << state_id
           << "_message_;\n";
  }
  header << "    \n";

  // Add parser instances for each read transition
  header << "    // Parsers for incoming messages\n";
  for (const auto *rt : read_trans) {
    header << "    " << rt->identifier << "Parser " << rt->identifier
           << "_parser_;\n";
  }

  // Add serializer instances for each write transition
  header << "    \n";
  header << "    // Serializers for outgoing messages\n";
  for (const auto *wt : write_trans) {
    header << "    " << wt->identifier << "Serializer " << wt->identifier
           << "_serializer_;\n";
  }

  header << "};\n\n";
}

// Generate the constructor implementation
void generate_state_machine_constructor(std::ostringstream &source,
                                        const std::string &class_name,
                                        const ProtocolInfo &info) {
  source << class_name << "::" << class_name << "() {\n";
  source << "    // Initialize parsers and serializers\n";
  source << "}\n\n";
}

// Generate the on_bytes_received implementation
void generate_on_bytes_received(std::ostringstream &source,
                                const std::string &class_name,
                                const ProtocolInfo &info,
                                bool is_client) {
  auto read_trans = get_read_transitions_for_agent(info, is_client);

  source << "size_t " << class_name
         << "::on_bytes_received(std::string_view data) {\n";
  source << "    if (data.empty() || is_closed()) {\n";
  source << "        return 0;\n";
  source << "    }\n";
  source << "    \n";
  source << "    size_t total_consumed = 0;\n";
  source << "    \n";
  source << "    // Dispatch to appropriate parser based on current state\n";
  source << "    switch (current_state_) {\n";

  // Group read transitions by when_state
  std::map<std::string, std::vector<const ReadTransitionInfo *>> by_state;
  for (const auto *rt : read_trans) {
    by_state[rt->when_state].push_back(rt);
  }

  for (const auto &[state, transitions] : by_state) {
    source << "    case State::" << state_name_to_identifier(state) << ": {\n";

    // Try all possible parsers for this state
    // Each parser is tried in sequence, using the first one that completes
    if (transitions.size() == 1) {
      // Single transition - straightforward case
      const auto *rt = transitions[0];
      std::string then_state_id = state_name_to_identifier(rt->then_state);
      source << "        auto result = " << rt->identifier
             << "_parser_.parse(data);\n";
      source << "        total_consumed = result.consumed;\n";
      source << "        if (result.status == ParseStatus::Complete) {\n";
      source << "            pending_" << then_state_id << "_message_ = "
             << rt->identifier << "_parser_.take_data();\n";
      source << "            has_message_ = true;\n";
      source << "            message_state_ = State::" << then_state_id << ";\n";
      source << "            current_state_ = State::" << then_state_id << ";\n";
      source << "            " << rt->identifier << "_parser_.reset();\n";
      source << "        }\n";
    } else if (!transitions.empty()) {
      // Multiple transitions - try each parser in parallel
      // Don't reset parsers at start - they may have buffered data from previous calls
      // We try all parsers and use the first one that completes
      // If one returns Error immediately, we try the next
      // If one is making progress (consumed > 0 or NeedMoreData with partial match), we continue with it
      
      source << "        \n";
      
      for (size_t i = 0; i < transitions.size(); ++i) {
        const auto *rt = transitions[i];
        std::string then_state_id = state_name_to_identifier(rt->then_state);
        std::string result_var = "result_" + std::to_string(i);
        
        // Calculate indentation based on nesting level
        std::string indent(8 + i * 4, ' ');
        std::string body_indent(12 + i * 4, ' ');
        
        source << indent << "auto " << result_var << " = " << rt->identifier
               << "_parser_.parse(data);\n";
        source << indent << "if (" << result_var << ".status == ParseStatus::Complete) {\n";
        source << body_indent << "total_consumed = " << result_var << ".consumed;\n";
        source << body_indent << "pending_" << then_state_id << "_message_ = "
               << rt->identifier << "_parser_.take_data();\n";
        source << body_indent << "has_message_ = true;\n";
        source << body_indent << "message_state_ = State::" << then_state_id << ";\n";
        source << body_indent << "current_state_ = State::" << then_state_id << ";\n";
        // Reset all parsers after success
        for (const auto *rt2 : transitions) {
          source << body_indent << rt2->identifier << "_parser_.reset();\n";
        }
        if (i + 1 < transitions.size()) {
          // If this parser returned NeedMoreData with consumed bytes, keep them
          // Only try next parser if this one immediately returned Error with 0 consumed
          source << indent << "} else if (" << result_var << ".status == ParseStatus::NeedMoreData) {\n";
          source << body_indent << "// Parser needs more data - keep buffered state\n";
          source << body_indent << "total_consumed = " << result_var << ".consumed;\n";
          // Reset the other parsers that haven't been tried yet since we're committing to this one
          for (size_t j = i + 1; j < transitions.size(); ++j) {
            source << body_indent << transitions[j]->identifier << "_parser_.reset();\n";
          }
          source << indent << "} else {\n";
          // Parser returned Error - reset it and try next
          source << body_indent << rt->identifier << "_parser_.reset();\n";
        } else {
          source << indent << "} else if (" << result_var << ".status == ParseStatus::NeedMoreData) {\n";
          source << body_indent << "total_consumed = " << result_var << ".consumed;\n";
          source << indent << "}\n";
        }
      }
      // Close the else blocks (one less than number of transitions)
      for (size_t i = transitions.size() - 1; i > 0; --i) {
        std::string indent(8 + (i - 1) * 4, ' ');
        source << indent << "}\n";
      }
    }

    source << "        break;\n";
    source << "    }\n";
  }

  source << "    default:\n";
  source << "        break;\n";
  source << "    }\n";
  source << "    \n";
  source << "    return total_consumed;\n";
  source << "}\n\n";
}

// Generate state-specific take_message methods
void generate_take_message_methods(std::ostringstream &source,
                                   const std::string &class_name,
                                   const ProtocolInfo &info,
                                   bool is_client) {
  auto messages_by_then_state = get_messages_by_then_state(info, is_client);

  for (const auto &[state_name, transitions] : messages_by_then_state) {
    if (transitions.empty())
      continue;
    std::string state_id = state_name_to_identifier(state_name);

    source << state_id << "Input " << class_name << "::take_" << state_id
           << "_message() {\n";
    source << "    has_message_ = false;\n";
    source << "    auto result = std::move(*pending_" << state_id << "_message_);\n";
    source << "    pending_" << state_id << "_message_.reset();\n";
    source << "    return result;\n";
    source << "}\n\n";
  }
}

// Generate the bytes_written implementation
void generate_bytes_written(std::ostringstream &source,
                            const std::string &class_name) {
  source << "void " << class_name << "::bytes_written(size_t count) {\n";
  source << "    if (count >= output_buffer_.size()) {\n";
  source << "        output_buffer_.clear();\n";
  source << "    } else {\n";
  source << "        output_buffer_.erase(0, count);\n";
  source << "    }\n";
  source << "}\n\n";
}

// Generate the on_eof implementation
void generate_on_eof(std::ostringstream &source,
                     const std::string &class_name,
                     const ProtocolInfo &info,
                     bool is_client) {
  auto read_trans = get_read_transitions_for_agent(info, is_client);

  source << "void " << class_name << "::on_eof() {\n";
  source << "    if (is_closed()) {\n";
  source << "        return;\n";
  source << "    }\n";
  source << "    \n";
  source << "    // Notify active parsers of EOF\n";
  source << "    switch (current_state_) {\n";

  // Group read transitions by when_state
  std::map<std::string, std::vector<const ReadTransitionInfo *>> by_state;
  for (const auto *rt : read_trans) {
    by_state[rt->when_state].push_back(rt);
  }

  for (const auto &[state, transitions] : by_state) {
    source << "    case State::" << state_name_to_identifier(state) << ":\n";
    for (const auto *rt : transitions) {
      source << "        " << rt->identifier << "_parser_.on_eof();\n";
    }
    source << "        break;\n";
  }

  source << "    default:\n";
  source << "        break;\n";
  source << "    }\n";
  source << "}\n\n";
}

// Generate send methods for each write transition
void generate_send_methods(std::ostringstream &source,
                           const std::string &class_name,
                           const ProtocolInfo &info,
                           bool is_client) {
  auto write_trans = get_write_transitions_for_agent(info, is_client);

  for (const auto *wt : write_trans) {
    // const ref version
    source << "void " << class_name << "::send_" << wt->identifier << "(const "
           << wt->identifier << "Data& data) {\n";
    source << "    " << wt->identifier << "_serializer_.set_data(data);\n";
    source << "    while (!" << wt->identifier << "_serializer_.is_complete()) {\n";
    source << "        auto chunk = " << wt->identifier
           << "_serializer_.next_chunk();\n";
    source << "        if (chunk.empty()) break;\n";
    source << "        output_buffer_.append(chunk);\n";
    source << "        " << wt->identifier << "_serializer_.advance();\n";
    source << "    }\n";
    source << "    current_state_ = State::"
           << state_name_to_identifier(wt->then_state) << ";\n";
    source << "}\n\n";

    // move version
    source << "void " << class_name << "::send_" << wt->identifier << "("
           << wt->identifier << "Data&& data) {\n";
    source << "    " << wt->identifier << "_serializer_.set_data(std::move(data));\n";
    source << "    while (!" << wt->identifier << "_serializer_.is_complete()) {\n";
    source << "        auto chunk = " << wt->identifier
           << "_serializer_.next_chunk();\n";
    source << "        if (chunk.empty()) break;\n";
    source << "        output_buffer_.append(chunk);\n";
    source << "        " << wt->identifier << "_serializer_.advance();\n";
    source << "    }\n";
    source << "    current_state_ = State::"
           << state_name_to_identifier(wt->then_state) << ";\n";
    source << "}\n\n";
  }
}

} // anonymous namespace

StateMachineResult generate_state_machine(const OutputContext &ctx,
                                          const ProtocolInfo &info) {
  StateMachineResult result;
  std::ostringstream header;
  std::ostringstream source;
  std::string guard = ctx.header_guard("state_machine.hpp");

  // Header file
  header << "// This file is auto-generated. Do not edit.\n\n";
  header << "#ifndef " << guard << "\n";
  header << "#define " << guard << "\n";
  header << "\n";
  header << "#include <optional>\n";
  header << "#include <string>\n";
  header << "#include <string_view>\n";
  header << "#include \"data_types.hpp\"\n";
  header << "#include \"parser.hpp\"\n";
  header << "#include \"serializer.hpp\"\n";
  header << "#include \"states.hpp\"\n";
  header << "\n";
  header << ctx.open_namespace();
  header << "\n";

  // Generate ClientStateMachine
  generate_state_machine_class_header(header, "ClientStateMachine", info, true);

  // Generate ServerStateMachine
  generate_state_machine_class_header(header, "ServerStateMachine", info, false);

  header << ctx.close_namespace();
  header << "\n";
  header << "#endif // " << guard << "\n";

  // Source file
  source << "// This file is auto-generated. Do not edit.\n\n";
  source << "#include \"state_machine.hpp\"\n";
  source << "\n";
  source << ctx.open_namespace();
  source << "\n";

  // Generate ClientStateMachine implementation
  source << "// ClientStateMachine implementation\n";
  generate_state_machine_constructor(source, "ClientStateMachine", info);
  generate_on_bytes_received(source, "ClientStateMachine", info, true);
  generate_take_message_methods(source, "ClientStateMachine", info, true);
  generate_bytes_written(source, "ClientStateMachine");
  generate_on_eof(source, "ClientStateMachine", info, true);
  generate_send_methods(source, "ClientStateMachine", info, true);

  // Generate ServerStateMachine implementation
  source << "// ServerStateMachine implementation\n";
  generate_state_machine_constructor(source, "ServerStateMachine", info);
  generate_on_bytes_received(source, "ServerStateMachine", info, false);
  generate_take_message_methods(source, "ServerStateMachine", info, false);
  generate_bytes_written(source, "ServerStateMachine");
  generate_on_eof(source, "ServerStateMachine", info, false);
  generate_send_methods(source, "ServerStateMachine", info, false);

  source << ctx.close_namespace();

  result.header = header.str();
  result.source = source.str();
  return result;
}

} // namespace networkprotocoldsl::codegen
