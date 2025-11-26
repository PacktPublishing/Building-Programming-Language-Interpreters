#include <networkprotocoldsl/codegen/protocolinfo.hpp>
#include <networkprotocoldsl/codegen/typemapping.hpp>

#include <unordered_set>

namespace networkprotocoldsl::codegen {

ProtocolInfo::ProtocolInfo(std::shared_ptr<const sema::ast::Protocol> protocol)
    : protocol_(std::move(protocol)) {
  collect_states();
  collect_messages();
  collect_transitions();
}

void ProtocolInfo::collect_states() {
  // Collect from client agent
  if (protocol_->client) {
    for (const auto &[state_name, state] : protocol_->client->states) {
      states_.insert(state_name);
      // Also collect target states from transitions
      for (const auto &[msg_name, transition_pair] : state->transitions) {
        states_.insert(transition_pair.second); // target state
      }
    }
  }

  // Collect from server agent
  if (protocol_->server) {
    for (const auto &[state_name, state] : protocol_->server->states) {
      states_.insert(state_name);
      for (const auto &[msg_name, transition_pair] : state->transitions) {
        states_.insert(transition_pair.second);
      }
    }
  }
}

void ProtocolInfo::collect_messages() {
  std::unordered_set<std::string> seen_messages;

  auto process_agent =
      [&](const std::shared_ptr<const sema::ast::Agent> &agent,
          const std::string &agent_name) {
        if (!agent)
          return;

        for (const auto &[state_name, state] : agent->states) {
          for (const auto &[msg_name, transition_pair] : state->transitions) {
            const auto &transition = transition_pair.first;
            const auto &target_state = transition_pair.second;

            // Check if we already have this message (same name) - O(1) lookup
            if (!seen_messages.insert(msg_name).second) {
              continue; // Already seen
            }

            // Extract data from transition
            std::shared_ptr<const parser::tree::MessageData> data;
            std::visit(
                [&](const auto &t) {
                  if (t) {
                    data = t->data;
                  }
                },
                transition);

            MessageInfo info;
            info.name = msg_name;
            info.identifier = message_name_to_identifier(msg_name);
            info.when_state = state_name;
            info.then_state = target_state;
            info.agent = agent_name;
            info.data = data;
            messages_.push_back(info);
          }
        }
      };

  process_agent(protocol_->client, "Client");
  process_agent(protocol_->server, "Server");
}

void ProtocolInfo::collect_transitions() {
  std::unordered_set<std::string> seen_read_transitions;
  std::unordered_set<std::string> seen_write_transitions;

  auto process_agent =
      [&](const std::shared_ptr<const sema::ast::Agent> &agent,
          const std::string &agent_name) {
        if (!agent)
          return;

        for (const auto &[state_name, state] : agent->states) {
          for (const auto &[msg_name, transition_pair] : state->transitions) {
            const auto &transition = transition_pair.first;
            const auto &target_state = transition_pair.second;

            std::visit(
                [&](const auto &t) {
                  if (!t)
                    return;

                  using T = std::decay_t<decltype(*t)>;
                  if constexpr (std::is_same_v<T, sema::ast::ReadTransition>) {
                    // Check if we already have this read transition - O(1) lookup
                    if (!seen_read_transitions.insert(msg_name).second) {
                      return; // Already seen
                    }
                    ReadTransitionInfo info;
                    info.message_name = msg_name;
                    info.identifier = message_name_to_identifier(msg_name);
                    info.when_state = state_name;
                    info.then_state = target_state;
                    info.agent = agent_name;
                    info.actions = t->actions;
                    info.data = t->data;
                    read_transitions_.push_back(info);
                  } else if constexpr (std::is_same_v<T,
                                                      sema::ast::WriteTransition>) {
                    // Check if we already have this write transition - O(1) lookup
                    if (!seen_write_transitions.insert(msg_name).second) {
                      return; // Already seen
                    }
                    WriteTransitionInfo info;
                    info.message_name = msg_name;
                    info.identifier = message_name_to_identifier(msg_name);
                    info.when_state = state_name;
                    info.then_state = target_state;
                    info.agent = agent_name;
                    info.actions = t->actions;
                    info.data = t->data;
                    write_transitions_.push_back(info);
                  }
                },
                transition);
          }
        }
      };

  process_agent(protocol_->client, "Client");
  process_agent(protocol_->server, "Server");
}

} // namespace networkprotocoldsl::codegen
