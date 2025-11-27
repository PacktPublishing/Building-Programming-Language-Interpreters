#include <networkprotocoldsl/codegen/generate_states.hpp>
#include <networkprotocoldsl/codegen/typemapping.hpp>

#include <map>
#include <sstream>

namespace networkprotocoldsl::codegen {

StatesResult generate_states(const OutputContext &ctx,
                             const ProtocolInfo &info) {
  StatesResult result;
  std::ostringstream header;
  std::ostringstream source;
  std::string guard = ctx.header_guard("states.hpp");

  const auto &states = info.states();
  const auto &messages = info.messages();

  // Header file
  header << "#ifndef " << guard << "\n";
  header << "#define " << guard << "\n";
  header << "\n";
  header << "#include <variant>\n";
  header << "#include \"data_types.hpp\"\n";
  header << "\n";
  header << ctx.open_namespace();
  header << "\n";

  // Generate State enum
  header << "enum class State {\n";
  for (const auto &state : states) {
    header << "    " << state_name_to_identifier(state) << ",\n";
  }
  header << "};\n\n";

  // Group messages by their source state (when) for outputs
  // and by their target state (then) for inputs
  std::map<std::string, std::vector<const MessageInfo *>> messages_from_state;
  std::map<std::string, std::vector<const MessageInfo *>> messages_to_state;

  for (const auto &msg : messages) {
    messages_from_state[msg.when_state].push_back(&msg);
    messages_to_state[msg.then_state].push_back(&msg);
  }

  // Generate transition output types for each state
  // These are the possible messages that can be sent when leaving a state
  for (const auto &[state_name, state_messages] : messages_from_state) {
    if (state_messages.empty())
      continue;

    header << "// Possible transitions from state " << state_name << "\n";
    header << "using " << state_name_to_identifier(state_name)
           << "Output = std::variant<\n";
    for (size_t i = 0; i < state_messages.size(); ++i) {
      header << "    " << state_messages[i]->identifier << "Data";
      if (i < state_messages.size() - 1)
        header << ",";
      header << "\n";
    }
    header << ">;\n\n";
  }

  // Generate transition input types for each state
  // These are the possible messages that can be received when entering a state
  for (const auto &[state_name, state_messages] : messages_to_state) {
    if (state_messages.empty())
      continue;

    header << "// Possible inputs when entering state " << state_name << "\n";
    header << "using " << state_name_to_identifier(state_name)
           << "Input = std::variant<\n";
    for (size_t i = 0; i < state_messages.size(); ++i) {
      header << "    " << state_messages[i]->identifier << "Data";
      if (i < state_messages.size() - 1)
        header << ",";
      header << "\n";
    }
    header << ">;\n\n";
  }

  header << ctx.close_namespace();
  header << "\n";
  header << "#endif // " << guard << "\n";

  // Source file
  source << "#include \"states.hpp\"\n";
  source << "\n";
  source << ctx.open_namespace();
  source << "\n";
  source << "// State-related implementations (if needed)\n";
  source << "\n";
  source << ctx.close_namespace();

  result.header = header.str();
  result.source = source.str();
  return result;
}

} // namespace networkprotocoldsl::codegen
