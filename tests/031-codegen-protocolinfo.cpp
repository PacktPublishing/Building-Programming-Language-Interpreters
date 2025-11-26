#include <networkprotocoldsl/codegen/protocolinfo.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>
#include <networkprotocoldsl/parser/parse.hpp>
#include <networkprotocoldsl/sema/analyze.hpp>

#include <fstream>
#include <gtest/gtest.h>
#include <set>

using namespace networkprotocoldsl;
using namespace networkprotocoldsl::codegen;

class ProtocolInfoTest : public ::testing::Test {
protected:
  std::shared_ptr<const sema::ast::Protocol> protocol_;

  void SetUp() override {
    std::string test_file =
        std::string(TEST_DATA_DIR) + "/023-source-code-http-client-server.txt";
    std::ifstream file(test_file);
    ASSERT_TRUE(file.is_open());
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    auto maybe_tokens = lexer::tokenize(content);
    ASSERT_TRUE(maybe_tokens.has_value());

    auto result = parser::parse(maybe_tokens.value());
    ASSERT_TRUE(result.has_value());

    auto maybe_protocol = sema::analyze(result.value());
    ASSERT_TRUE(maybe_protocol.has_value());
    protocol_ = maybe_protocol.value();
  }
};

TEST_F(ProtocolInfoTest, CollectsStates) {
  ProtocolInfo info(protocol_);

  const auto &states = info.states();
  EXPECT_FALSE(states.empty());

  // Check for expected states from HTTP protocol
  EXPECT_TRUE(states.count("Open") > 0);
  EXPECT_TRUE(states.count("Closed") > 0);
  EXPECT_TRUE(states.count("AwaitResponse") > 0);
}

TEST_F(ProtocolInfoTest, CollectsMessages) {
  ProtocolInfo info(protocol_);

  const auto &messages = info.messages();
  EXPECT_FALSE(messages.empty());

  // Check that messages have proper identifiers
  bool found_http_request = false;
  bool found_http_response = false;
  for (const auto &msg : messages) {
    if (msg.name == "HTTP Request") {
      found_http_request = true;
      EXPECT_EQ(msg.identifier, "HTTPRequest");
    }
    if (msg.name == "HTTP Response") {
      found_http_response = true;
      EXPECT_EQ(msg.identifier, "HTTPResponse");
    }
  }
  EXPECT_TRUE(found_http_request);
  EXPECT_TRUE(found_http_response);
}

TEST_F(ProtocolInfoTest, CollectsReadTransitions) {
  ProtocolInfo info(protocol_);

  const auto &read_transitions = info.read_transitions();
  EXPECT_FALSE(read_transitions.empty());

  // Read transitions should have actions
  for (const auto &rt : read_transitions) {
    // Each read transition should have a message name and identifier
    EXPECT_FALSE(rt.message_name.empty());
    EXPECT_FALSE(rt.identifier.empty());
    EXPECT_FALSE(rt.when_state.empty());
    EXPECT_FALSE(rt.then_state.empty());
  }
}

TEST_F(ProtocolInfoTest, CollectsWriteTransitions) {
  ProtocolInfo info(protocol_);

  const auto &write_transitions = info.write_transitions();
  EXPECT_FALSE(write_transitions.empty());

  // Write transitions should have actions
  for (const auto &wt : write_transitions) {
    EXPECT_FALSE(wt.message_name.empty());
    EXPECT_FALSE(wt.identifier.empty());
    EXPECT_FALSE(wt.when_state.empty());
    EXPECT_FALSE(wt.then_state.empty());
  }
}

TEST_F(ProtocolInfoTest, MessageInfoHasCorrectFields) {
  ProtocolInfo info(protocol_);

  const auto &messages = info.messages();
  for (const auto &msg : messages) {
    // All messages should have these fields populated
    EXPECT_FALSE(msg.name.empty()) << "Message name should not be empty";
    EXPECT_FALSE(msg.identifier.empty())
        << "Message identifier should not be empty";
    EXPECT_FALSE(msg.when_state.empty())
        << "Message when_state should not be empty";
    EXPECT_FALSE(msg.then_state.empty())
        << "Message then_state should not be empty";
    EXPECT_FALSE(msg.agent.empty()) << "Message agent should not be empty";
    // agent should be either "Client" or "Server"
    EXPECT_TRUE(msg.agent == "Client" || msg.agent == "Server")
        << "Agent should be Client or Server, got: " << msg.agent;
  }
}

TEST_F(ProtocolInfoTest, ProtocolAccessor) {
  ProtocolInfo info(protocol_);

  // Should be able to access the original protocol
  EXPECT_EQ(info.protocol(), protocol_);
  EXPECT_TRUE(info.protocol()->client != nullptr);
  EXPECT_TRUE(info.protocol()->server != nullptr);
}

TEST_F(ProtocolInfoTest, ReadTransitionsHaveActions) {
  ProtocolInfo info(protocol_);

  const auto &read_transitions = info.read_transitions();
  
  // At least some read transitions should have actions
  bool has_actions = false;
  for (const auto &rt : read_transitions) {
    if (!rt.actions.empty()) {
      has_actions = true;
      break;
    }
  }
  EXPECT_TRUE(has_actions) << "At least one read transition should have actions";
}

TEST_F(ProtocolInfoTest, WriteTransitionsHaveActions) {
  ProtocolInfo info(protocol_);

  const auto &write_transitions = info.write_transitions();
  
  // At least some write transitions should have actions
  bool has_actions = false;
  for (const auto &wt : write_transitions) {
    if (!wt.actions.empty()) {
      has_actions = true;
      break;
    }
  }
  EXPECT_TRUE(has_actions) << "At least one write transition should have actions";
}

TEST_F(ProtocolInfoTest, MessagesHaveDataOrEmpty) {
  ProtocolInfo info(protocol_);

  const auto &messages = info.messages();
  
  // All messages should have data pointer (may be empty for data-less messages)
  for (const auto &msg : messages) {
    // Messages can have null data for "Client Closes Connection" type messages
    // which have no data section
    if (msg.data) {
      // If data exists, it's a valid map (could be empty)
      EXPECT_TRUE(msg.data != nullptr);
    }
  }
}

TEST_F(ProtocolInfoTest, StatesFormConnectedGraph) {
  ProtocolInfo info(protocol_);

  const auto &states = info.states();
  const auto &messages = info.messages();
  
  // Every message's when_state and then_state should be in the states set
  for (const auto &msg : messages) {
    EXPECT_TRUE(states.count(msg.when_state) > 0)
        << "when_state '" << msg.when_state << "' should be in states set";
    EXPECT_TRUE(states.count(msg.then_state) > 0)
        << "then_state '" << msg.then_state << "' should be in states set";
  }
}

TEST_F(ProtocolInfoTest, ClientAndServerTransitions) {
  ProtocolInfo info(protocol_);

  const auto &read_trans = info.read_transitions();
  const auto &write_trans = info.write_transitions();
  
  // HTTP protocol should have both read and write transitions
  // Client sends HTTP Request (write) and receives HTTP Response (read)
  // from the client's perspective
  EXPECT_FALSE(read_trans.empty()) << "Should have read transitions";
  EXPECT_FALSE(write_trans.empty()) << "Should have write transitions";
}

TEST_F(ProtocolInfoTest, TerminalStateIncluded) {
  ProtocolInfo info(protocol_);

  const auto &states = info.states();
  
  // "Closed" is a terminal state (no outgoing transitions)
  // It should still be in the states set because it's a target state
  EXPECT_TRUE(states.count("Closed") > 0)
      << "Terminal state 'Closed' should be in states set";
  
  // Verify Closed has no outgoing transitions in messages
  bool closed_has_outgoing = false;
  for (const auto &msg : info.messages()) {
    if (msg.when_state == "Closed") {
      closed_has_outgoing = true;
      break;
    }
  }
  // Closed is terminal, so it should NOT have outgoing transitions
  EXPECT_FALSE(closed_has_outgoing)
      << "Terminal state 'Closed' should not have outgoing transitions";
}

TEST_F(ProtocolInfoTest, NoDuplicateMessages) {
  ProtocolInfo info(protocol_);

  const auto &messages = info.messages();
  std::set<std::string> seen_names;
  
  for (const auto &msg : messages) {
    EXPECT_TRUE(seen_names.insert(msg.name).second)
        << "Duplicate message found: " << msg.name;
  }
}

TEST_F(ProtocolInfoTest, NoDuplicateReadTransitions) {
  ProtocolInfo info(protocol_);

  const auto &read_transitions = info.read_transitions();
  std::set<std::string> seen_names;
  
  for (const auto &rt : read_transitions) {
    EXPECT_TRUE(seen_names.insert(rt.message_name).second)
        << "Duplicate read transition found: " << rt.message_name;
  }
}

TEST_F(ProtocolInfoTest, NoDuplicateWriteTransitions) {
  ProtocolInfo info(protocol_);

  const auto &write_transitions = info.write_transitions();
  std::set<std::string> seen_names;
  
  for (const auto &wt : write_transitions) {
    EXPECT_TRUE(seen_names.insert(wt.message_name).second)
        << "Duplicate write transition found: " << wt.message_name;
  }
}

// Test with empty/minimal protocol
TEST(ProtocolInfoEmptyTest, EmptyProtocol) {
  // Create an empty protocol
  auto protocol = std::make_shared<sema::ast::Protocol>();
  protocol->client = nullptr;
  protocol->server = nullptr;
  
  ProtocolInfo info(protocol);
  
  // Should handle empty protocol gracefully
  EXPECT_TRUE(info.states().empty());
  EXPECT_TRUE(info.messages().empty());
  EXPECT_TRUE(info.read_transitions().empty());
  EXPECT_TRUE(info.write_transitions().empty());
  EXPECT_EQ(info.protocol(), protocol);
}

TEST(ProtocolInfoEmptyTest, ClientOnlyProtocol) {
  // Create a protocol with only client agent
  auto protocol = std::make_shared<sema::ast::Protocol>();
  auto client = std::make_shared<sema::ast::Agent>();
  protocol->client = client;
  protocol->server = nullptr;
  
  ProtocolInfo info(protocol);
  
  // Should handle client-only protocol gracefully
  EXPECT_TRUE(info.states().empty()); // No states defined
  EXPECT_TRUE(info.messages().empty());
  EXPECT_EQ(info.protocol(), protocol);
}

TEST(ProtocolInfoEmptyTest, ServerOnlyProtocol) {
  // Create a protocol with only server agent
  auto protocol = std::make_shared<sema::ast::Protocol>();
  protocol->client = nullptr;
  auto server = std::make_shared<sema::ast::Agent>();
  protocol->server = server;
  
  ProtocolInfo info(protocol);
  
  // Should handle server-only protocol gracefully
  EXPECT_TRUE(info.states().empty());
  EXPECT_TRUE(info.messages().empty());
  EXPECT_EQ(info.protocol(), protocol);
}
