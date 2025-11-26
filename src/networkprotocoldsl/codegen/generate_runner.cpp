#include <networkprotocoldsl/codegen/generate_runner.hpp>
#include <networkprotocoldsl/codegen/typemapping.hpp>

#include <map>
#include <set>
#include <sstream>

namespace networkprotocoldsl::codegen {

namespace {

// Get read transitions for an agent, grouped by then_state
std::map<std::string, std::vector<const ReadTransitionInfo *>>
get_read_transitions_by_then_state(const ProtocolInfo &info, bool is_client) {
  std::map<std::string, std::vector<const ReadTransitionInfo *>> result;
  std::string agent = is_client ? "Client" : "Server";
  for (const auto &rt : info.read_transitions()) {
    if (rt.agent == agent) {
      result[rt.then_state].push_back(&rt);
    }
  }
  return result;
}

// Get write transitions for an agent, grouped by when_state
std::map<std::string, std::vector<const WriteTransitionInfo *>>
get_write_transitions_by_when_state(const ProtocolInfo &info, bool is_client) {
  std::map<std::string, std::vector<const WriteTransitionInfo *>> result;
  std::string agent = is_client ? "Client" : "Server";
  for (const auto &wt : info.write_transitions()) {
    if (wt.agent == agent) {
      result[wt.when_state].push_back(&wt);
    }
  }
  return result;
}

// Find the initial state's output type (for on_connection)
std::optional<std::string> get_initial_state_output_type(const ProtocolInfo &info, bool is_client) {
  auto by_when_state = get_write_transitions_by_when_state(info, is_client);
  auto it = by_when_state.find("Open");
  if (it != by_when_state.end() && !it->second.empty()) {
    return "OpenOutput";
  }
  return std::nullopt;
}

// Generate the concept for handler callbacks
// Each handler has one method per state, with overloads for each message type
// that can arrive at that state. ADL resolves which overload to call.
void generate_handler_concept(std::ostringstream &header,
                              const std::string &concept_name,
                              const std::string &prefix,
                              const ProtocolInfo &info,
                              bool is_client) {
  auto by_then_state = get_read_transitions_by_then_state(info, is_client);
  auto by_when_state = get_write_transitions_by_when_state(info, is_client);
  
  // Check if Open state has outputs (initial message to send)
  bool has_open_output = by_when_state.find("Open") != by_when_state.end() &&
                         !by_when_state.at("Open").empty();

  header << "/**\n";
  header << " * Concept for " << prefix << " protocol handler.\n";
  header << " *\n";
  header << " * IMPORTANT: The handler instance is shared across ALL connections.\n";
  header << " * Handler implementations must NOT store per-connection state as member\n";
  header << " * variables. Instead, all connection-specific data must be carried in the\n";
  header << " * input message data and returned in the output message data.\n";
  header << " *\n";
  header << " * ALL HANDLER METHODS MUST BE CONST to enforce this at compile-time.\n";
  header << " *\n";
  header << " * The handler is appropriate for storing shared configuration, database\n";
  header << " * connections, or other resources that are common to all connections.\n";
  header << " *\n";
  header << " * Implementations must provide handler methods for each state where\n";
  header << " * messages are received. Each method is named on_<StateName> and has\n";
  header << " * overloads for each message type that can arrive at that state.\n";
  header << " * The runner uses std::visit + ADL to dispatch to the correct overload.\n";
  header << " *\n";
  header << " * Required method overloads (all must be const):\n";

  // Include on_Open if there are outputs from Open state
  if (has_open_output) {
    header << " *   OpenOutput on_Open() const\n";
  }

  // For each state, list the message types that can arrive there
  for (const auto &[state_name, transitions] : by_then_state) {
    std::string state_id = state_name_to_identifier(state_name);
    
    // Find the output type for this state
    auto it = by_when_state.find(state_name);
    if (it == by_when_state.end() || it->second.empty()) {
      continue;
    }
    
    std::string output_type = state_id + "Output";
    for (const auto &rt : transitions) {
      header << " *   " << output_type << " on_" << state_id << "(const " << rt->identifier << "Data&) const\n";
    }
  }

  header << " */\n";
  header << "template <typename T>\n";
  header << "concept " << concept_name << " = requires(const T& handler";

  // Add parameters for each message type
  std::string agent = is_client ? "Client" : "Server";
  for (const auto &rt : info.read_transitions()) {
    if (rt.agent != agent)
      continue;
    header << ", const " << rt.identifier << "Data& " << rt.identifier << "_msg";
  }

  header << ") {\n";

  // Include on_Open requirement if needed
  if (has_open_output) {
    header << "    { handler.on_Open() } -> std::convertible_to<OpenOutput>;\n";
  }

  // Generate requirements for each state's handler overloads
  for (const auto &[state_name, transitions] : by_then_state) {
    std::string state_id = state_name_to_identifier(state_name);
    
    // Find the output type for this state
    auto it = by_when_state.find(state_name);
    if (it == by_when_state.end() || it->second.empty()) {
      continue;
    }
    
    std::string output_type = state_id + "Output";
    
    // Each message type that transitions to this state needs an overload
    for (const auto &rt : transitions) {
      header << "    { handler.on_" << state_id << "(" << rt->identifier << "_msg) } -> std::convertible_to<" << output_type << ">;\n";
    }
  }

  header << "};\n\n";
}

// Generate the Runner class header
void generate_runner_class_header(std::ostringstream &header,
                                  const std::string &class_name,
                                  const std::string &state_machine_class,
                                  const std::string &handler_concept,
                                  const std::string &prefix,
                                  const ProtocolInfo &info,
                                  bool is_client) {
  auto by_then_state = get_read_transitions_by_then_state(info, is_client);
  auto by_when_state = get_write_transitions_by_when_state(info, is_client);
  
  // Check if Open state has outputs (initial message to send)
  bool has_open_output = by_when_state.find("Open") != by_when_state.end() &&
                         !by_when_state.at("Open").empty();

  header << "/**\n";
  header << " * " << class_name << " - Template-based runner for "
         << (is_client ? "client" : "server") << " side.\n";
  header << " *\n";
  header << " * This class wraps " << state_machine_class << " and dispatches\n";
  header << " * received messages to the appropriate handler method based on state.\n";
  header << " *\n";
  header << " * IMPORTANT: Each Runner instance manages one connection's state machine,\n";
  header << " * but the Handler reference is typically shared across all connections.\n";
  header << " * The Handler should only contain shared resources (configuration, etc.)\n";
  header << " * and must NOT store per-connection state. All connection-specific data\n";
  header << " * must flow through the message input/output parameters.\n";
  header << " *\n";
  header << " * All handler methods are required to be const to enforce this constraint.\n";
  header << " *\n";
  header << " * @tparam Handler A type satisfying the " << handler_concept << " concept.\n";
  header << " *\n";
  header << " * Usage:\n";
  header << " *   // Handler holds shared config, NOT per-connection state\n";
  header << " *   struct My" << prefix << "Handler {\n";
  header << " *       const Config& config;  // Shared across connections - OK\n";
  header << " *       // int request_count;  // Per-connection state - NOT OK\n";

  if (has_open_output) {
    header << " *       OpenOutput on_Open() const;\n";
  }

  // Document handlers for each state with overloads
  for (const auto &[state_name, transitions] : by_then_state) {
    std::string state_id = state_name_to_identifier(state_name);
    
    auto it = by_when_state.find(state_name);
    if (it == by_when_state.end() || it->second.empty()) {
      continue;
    }
    
    std::string output_type = state_id + "Output";
    for (const auto &rt : transitions) {
      header << " *       " << output_type << " on_" << state_id << "(const " << rt->identifier << "Data& msg) const;\n";
    }
  }

  header << " *   };\n";
  header << " *\n";
  header << " *   My" << prefix << "Handler handler;  // Shared across connections\n";
  header << " *   // Each connection gets its own Runner instance\n";
  header << " *   " << class_name << "<My" << prefix << "Handler> runner(handler);\n";
  if (has_open_output) {
    header << " *   runner.start();  // Call on_Open and send initial message\n";
  }
  header << " *   runner.on_bytes_received(data);\n";
  header << " */\n";
  header << "template <" << handler_concept << " Handler>\n";
  header << "class " << class_name << " {\n";
  header << "public:\n";
  header << "    explicit " << class_name << "(const Handler& handler) : handler_(handler) {}\n";
  header << "    \n";

  // Generate start() method if Open has outputs
  if (has_open_output) {
    header << "    // Call on_Open handler and dispatch initial message\n";
    header << "    void start() {\n";
    header << "        auto output = handler_.on_Open();\n";
    header << "        dispatch_output(output);\n";
    header << "    }\n";
    header << "    \n";
  }

  header << "    // Process received bytes, invoke handler on complete messages\n";
  header << "    size_t on_bytes_received(std::string_view data) {\n";
  header << "        size_t consumed = state_machine_.on_bytes_received(data);\n";
  header << "        \n";
  header << "        while (state_machine_.has_message()) {\n";
  header << "            dispatch_message();\n";
  header << "        }\n";
  header << "        \n";
  header << "        return consumed;\n";
  header << "    }\n";
  header << "    \n";
  header << "    // Check if there's output data pending\n";
  header << "    bool has_pending_output() const { return state_machine_.has_pending_output(); }\n";
  header << "    \n";
  header << "    // Get pending output bytes\n";
  header << "    std::string_view pending_output() const { return state_machine_.pending_output(); }\n";
  header << "    \n";
  header << "    // Mark bytes as written\n";
  header << "    void bytes_written(size_t count) { state_machine_.bytes_written(count); }\n";
  header << "    \n";
  header << "    // Check if connection is closed\n";
  header << "    bool is_closed() const { return state_machine_.is_closed(); }\n";
  header << "    \n";
  header << "    // Get current state\n";
  header << "    State current_state() const { return state_machine_.current_state(); }\n";
  header << "    \n";
  header << "    // Access the underlying state machine\n";
  header << "    " << state_machine_class << "& state_machine() { return state_machine_; }\n";
  header << "    const " << state_machine_class << "& state_machine() const { return state_machine_; }\n";
  header << "\n";
  header << "private:\n";
  header << "    " << state_machine_class << " state_machine_;\n";
  header << "    const Handler& handler_;\n";
  header << "    \n";
  header << "    void dispatch_message() {\n";
  header << "        State msg_state = state_machine_.message_state();\n";
  header << "        switch (msg_state) {\n";

  for (const auto &[state_name, transitions] : by_then_state) {
    std::string state_id = state_name_to_identifier(state_name);
    
    // Check if this state has outputs
    auto it = by_when_state.find(state_name);
    if (it == by_when_state.end() || it->second.empty()) {
      continue;
    }

    std::string output_type = state_id + "Output";
    
    header << "        case State::" << state_id << ": {\n";
    header << "            auto input = state_machine_.take_" << state_id << "_message();\n";
    // Use ADL to dispatch to the correct on_StateName overload based on message type
    header << "            auto output = std::visit([this](auto&& msg) -> " << output_type << " {\n";
    header << "                return handler_.on_" << state_id << "(msg);\n";
    header << "            }, input);\n";
    header << "            dispatch_output(output);\n";
    header << "            break;\n";
    header << "        }\n";
  }

  header << "        default:\n";
  header << "            break;\n";
  header << "        }\n";
  header << "    }\n";
  header << "    \n";

  // Generate dispatch_output for this state machine's output types
  header << "    template <typename Output>\n";
  header << "    void dispatch_output(const Output& output) {\n";
  header << "        std::visit([this](auto&& msg) {\n";
  header << "            using T = std::decay_t<decltype(msg)>;\n";

  std::string agent = is_client ? "Client" : "Server";
  bool first = true;
  for (const auto &wt : info.write_transitions()) {
    if (wt.agent != agent)
      continue;

    header << "            ";
    if (!first)
      header << "else ";
    header << "if constexpr (std::is_same_v<T, " << wt.identifier << "Data>) {\n";
    header << "                state_machine_.send_" << wt.identifier << "(msg);\n";
    header << "            }\n";
    first = false;
  }

  header << "        }, output);\n";
  header << "    }\n";
  header << "};\n\n";
}

} // anonymous namespace

RunnerResult generate_runner(const OutputContext &ctx,
                             const ProtocolInfo &info) {
  RunnerResult result;
  std::ostringstream header;
  std::ostringstream source;
  std::string guard = ctx.header_guard("runner.hpp");

  // Header file
  header << "#ifndef " << guard << "\n";
  header << "#define " << guard << "\n";
  header << "\n";
  header << "#include <concepts>\n";
  header << "#include <variant>\n";
  header << "#include \"data_types.hpp\"\n";
  header << "#include \"states.hpp\"\n";
  header << "#include \"state_machine.hpp\"\n";
  header << "\n";
  header << ctx.open_namespace();
  header << "\n";

  // Generate concept for Server handler
  generate_handler_concept(header, "ServerHandlerConcept", "Server", info, false);

  // Generate concept for Client handler
  generate_handler_concept(header, "ClientHandlerConcept", "Client", info, true);

  // Generate ServerRunner class (template, so all in header)
  generate_runner_class_header(header, "ServerRunner", "ServerStateMachine",
                               "ServerHandlerConcept", "Server", info, false);

  // Generate ClientRunner class (template, so all in header)
  generate_runner_class_header(header, "ClientRunner", "ClientStateMachine",
                               "ClientHandlerConcept", "Client", info, true);

  header << ctx.close_namespace();
  header << "\n";
  header << "#endif // " << guard << "\n";

  // Source file - minimal since templates are in header
  source << "#include \"runner.hpp\"\n";
  source << "\n";
  source << ctx.open_namespace();
  source << "\n";
  source << "// Template implementations are in the header.\n";
  source << "\n";
  source << ctx.close_namespace();

  result.header = header.str();
  result.source = source.str();
  return result;
}

} // namespace networkprotocoldsl::codegen
