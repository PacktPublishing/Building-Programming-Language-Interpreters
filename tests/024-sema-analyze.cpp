#include <networkprotocoldsl/lexer/token.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>
#include <networkprotocoldsl/parser/parse.hpp>
#include <networkprotocoldsl/sema/analyze.hpp>
#include <networkprotocoldsl/sema/ast/transition.hpp>

#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <variant>
#include <vector>

using namespace networkprotocoldsl;

TEST(MessageTest, Message) {
  std::string test_file =
      std::string(TEST_DATA_DIR) + "/023-source-code-http-client-server.txt";
  std::ifstream file(test_file);
  ASSERT_TRUE(file.is_open());
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();
  auto maybe_tokens = lexer::tokenize(content);
  ASSERT_TRUE(maybe_tokens.has_value());
  std::vector<lexer::Token> &tokens = maybe_tokens.value();
  auto result = parser::parse(tokens);
  ASSERT_TRUE(result.has_value());
  auto protocol_description = result.value();

  auto maybe_protocol = sema::analyze(protocol_description);
  ASSERT_TRUE(maybe_protocol.has_value());
  auto protocol = maybe_protocol.value();
  ASSERT_TRUE(protocol->client);
  ASSERT_TRUE(protocol->server);

  auto client = protocol->client;
  auto server = protocol->server;

  auto &client_states = client->states;
  auto &server_states = server->states;

  ASSERT_EQ(client_states.size(), 3);
  ASSERT_EQ(server_states.size(), 3);

  auto client_open_state = client_states.at("Open");
  auto server_open_state = server_states.at("Open");
  auto &client_open_transitions = client_open_state->transitions;
  auto &server_open_transitions = server_open_state->transitions;

  ASSERT_EQ(client_open_transitions.size(), 2);

  auto &client_send_request_transition_pair =
      client_open_transitions.at("HTTP Request");
  ASSERT_EQ(client_send_request_transition_pair.second, "AwaitResponse");
  auto &client_send_request_transition =
      std::get<std::shared_ptr<const sema::ast::WriteTransition>>(
          client_send_request_transition_pair.first);
  ASSERT_EQ(10, client_send_request_transition->actions.size());
  const auto &client_send_request_actions =
      client_send_request_transition->actions;
  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
      client_send_request_actions[0]));
  auto write_from_identifier_0 =
      std::get<std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
          client_send_request_actions[0]);
  ASSERT_EQ(write_from_identifier_0->identifier->name, "verb");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
      client_send_request_actions[1]));
  auto write_static_octets_1 =
      std::get<std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
          client_send_request_actions[1]);
  ASSERT_EQ(write_static_octets_1->octets, " ");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
      client_send_request_actions[2]));
  auto write_from_identifier_2 =
      std::get<std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
          client_send_request_actions[2]);
  ASSERT_EQ(write_from_identifier_2->identifier->name, "request_target");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
      client_send_request_actions[3]));
  auto write_static_octets_3 =
      std::get<std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
          client_send_request_actions[3]);
  ASSERT_EQ(write_static_octets_3->octets, " ");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
      client_send_request_actions[4]));
  auto write_static_octets_client_4 =
      std::get<std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
          client_send_request_actions[4]);
  ASSERT_EQ(write_static_octets_client_4->octets, "HTTP/");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
      client_send_request_actions[5]));
  auto write_from_identifier_client_5 =
      std::get<std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
          client_send_request_actions[5]);
  ASSERT_EQ(write_from_identifier_client_5->identifier->name, "major_version");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
      client_send_request_actions[6]));
  auto write_static_octets_6 =
      std::get<std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
          client_send_request_actions[6]);
  ASSERT_EQ(write_static_octets_6->octets, ".");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
      client_send_request_actions[7]));
  auto write_from_identifier_7 =
      std::get<std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
          client_send_request_actions[7]);
  ASSERT_EQ(write_from_identifier_7->identifier->name, "minor_version");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
      client_send_request_actions[8]));
  auto write_static_octets_8 =
      std::get<std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
          client_send_request_actions[8]);
  ASSERT_EQ(write_static_octets_8->octets, "\r\n");

  ASSERT_TRUE(
      std::holds_alternative<std::shared_ptr<const sema::ast::action::Loop>>(
          client_send_request_actions[9]));
  auto client_loop_9 = std::get<std::shared_ptr<const sema::ast::action::Loop>>(
      client_send_request_actions[9]);
  ASSERT_EQ(client_loop_9->variable->name, "header");
  ASSERT_EQ(client_loop_9->collection->name, "headers");
  ASSERT_EQ(client_loop_9->terminator, "\r\n");
  ASSERT_EQ(client_loop_9->actions.size(), 4);
  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
      client_loop_9->actions[0]));
  auto client_loop_action_0 =
      std::get<std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
          client_loop_9->actions[0]);
  ASSERT_EQ(client_loop_action_0->identifier->name, "header");
  ASSERT_TRUE(client_loop_action_0->identifier->member.has_value());
  ASSERT_EQ(client_loop_action_0->identifier->member.value()->name, "key");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
      client_loop_9->actions[1]));
  auto client_loop_action_1 =
      std::get<std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
          client_loop_9->actions[1]);
  ASSERT_EQ(client_loop_action_1->octets, ":");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
      client_loop_9->actions[2]));
  auto client_loop_action_2 =
      std::get<std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
          client_loop_9->actions[2]);
  ASSERT_EQ(client_loop_action_2->identifier->name, "header");
  ASSERT_TRUE(client_loop_action_2->identifier->member.has_value());
  ASSERT_EQ(client_loop_action_2->identifier->member.value()->name, "value");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
      client_loop_9->actions[3]));
  auto client_loop_action_3 =
      std::get<std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
          client_loop_9->actions[3]);
  ASSERT_EQ(client_loop_action_3->octets, "\r\n");

  auto &client_closes_transition_pair =
      client_open_transitions.at("Client Closes Connection");
  ASSERT_EQ(client_closes_transition_pair.second, "Closed");
  auto &client_closes_transition =
      std::get<std::shared_ptr<const sema::ast::WriteTransition>>(
          client_closes_transition_pair.first);
  ASSERT_EQ(0, client_closes_transition->actions.size());

  ASSERT_EQ(server_open_transitions.size(), 2);

  auto &server_receive_request_transition_pair =
      server_open_transitions.at("HTTP Request");
  ASSERT_EQ(server_receive_request_transition_pair.second, "AwaitResponse");
  auto &server_receive_request_transition =
      std::get<std::shared_ptr<const sema::ast::ReadTransition>>(
          server_receive_request_transition_pair.first);
  ASSERT_EQ(6, server_receive_request_transition->actions.size());
  const auto &server_receive_request_actions =
      server_receive_request_transition->actions;
  ASSERT_TRUE(
      std::holds_alternative<
          std::shared_ptr<const sema::ast::action::ReadOctetsUntilTerminator>>(
          server_receive_request_actions[0]));
  auto read_octets_until_terminator_0 = std::get<
      std::shared_ptr<const sema::ast::action::ReadOctetsUntilTerminator>>(
      server_receive_request_actions[0]);
  ASSERT_EQ(read_octets_until_terminator_0->terminator, " ");
  ASSERT_EQ(read_octets_until_terminator_0->identifier->name, "verb");

  ASSERT_TRUE(
      std::holds_alternative<
          std::shared_ptr<const sema::ast::action::ReadOctetsUntilTerminator>>(
          server_receive_request_actions[1]));
  auto read_octets_until_terminator_1 = std::get<
      std::shared_ptr<const sema::ast::action::ReadOctetsUntilTerminator>>(
      server_receive_request_actions[1]);
  ASSERT_EQ(read_octets_until_terminator_1->terminator, " ");
  ASSERT_EQ(read_octets_until_terminator_1->identifier->name, "request_target");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::ReadStaticOctets>>(
      server_receive_request_actions[2]));

  ASSERT_TRUE(
      std::holds_alternative<
          std::shared_ptr<const sema::ast::action::ReadOctetsUntilTerminator>>(
          server_receive_request_actions[3]));
  auto read_octets_until_terminator_3 = std::get<
      std::shared_ptr<const sema::ast::action::ReadOctetsUntilTerminator>>(
      server_receive_request_actions[3]);
  ASSERT_EQ(read_octets_until_terminator_3->terminator, ".");
  ASSERT_EQ(read_octets_until_terminator_3->identifier->name, "major_version");

  ASSERT_TRUE(
      std::holds_alternative<
          std::shared_ptr<const sema::ast::action::ReadOctetsUntilTerminator>>(
          server_receive_request_actions[4]));
  auto read_octets_until_terminator_4 = std::get<
      std::shared_ptr<const sema::ast::action::ReadOctetsUntilTerminator>>(
      server_receive_request_actions[4]);
  ASSERT_EQ(read_octets_until_terminator_4->terminator, "\r\n");
  ASSERT_EQ(read_octets_until_terminator_4->identifier->name, "minor_version");

  ASSERT_TRUE(
      std::holds_alternative<std::shared_ptr<const sema::ast::action::Loop>>(
          server_receive_request_actions[5]));
  auto server_loop_5 = std::get<std::shared_ptr<const sema::ast::action::Loop>>(
      server_receive_request_actions[5]);
  ASSERT_EQ(server_loop_5->variable->name, "header");
  ASSERT_EQ(server_loop_5->collection->name, "headers");
  ASSERT_EQ(server_loop_5->terminator, "\r\n");
  ASSERT_EQ(server_loop_5->actions.size(), 2);
  ASSERT_TRUE(
      std::holds_alternative<
          std::shared_ptr<const sema::ast::action::ReadOctetsUntilTerminator>>(
          server_loop_5->actions[0]));
  auto server_loop_action_0 = std::get<
      std::shared_ptr<const sema::ast::action::ReadOctetsUntilTerminator>>(
      server_loop_5->actions[0]);
  ASSERT_EQ(server_loop_action_0->terminator, ":");
  ASSERT_EQ(server_loop_action_0->identifier->name, "header");
  ASSERT_TRUE(server_loop_action_0->identifier->member.has_value());
  ASSERT_EQ(server_loop_action_0->identifier->member.value()->name, "key");

  ASSERT_TRUE(
      std::holds_alternative<
          std::shared_ptr<const sema::ast::action::ReadOctetsUntilTerminator>>(
          server_loop_5->actions[1]));
  auto server_loop_action_1 = std::get<
      std::shared_ptr<const sema::ast::action::ReadOctetsUntilTerminator>>(
      server_loop_5->actions[1]);
  ASSERT_EQ(server_loop_action_1->terminator, "\r\n");
  ASSERT_EQ(server_loop_action_1->identifier->name, "header");
  ASSERT_TRUE(server_loop_action_1->identifier->member.has_value());
  ASSERT_EQ(server_loop_action_1->identifier->member.value()->name, "value");

  auto &server_closed_by_client_transition_pair =
      server_open_transitions.at("Client Closes Connection");
  ASSERT_EQ(server_closed_by_client_transition_pair.second, "Closed");
  auto &server_closed_by_client_transition =
      std::get<std::shared_ptr<const sema::ast::ReadTransition>>(
          server_closed_by_client_transition_pair.first);
  ASSERT_EQ(0, server_closed_by_client_transition->actions.size());

  auto client_closed_state = client_states.at("Closed");
  ASSERT_EQ(0, client_closed_state->transitions.size());
  auto server_closed_state = server_states.at("Closed");
  ASSERT_EQ(0, server_closed_state->transitions.size());

  auto &client_awaitresponse_state = client_states.at("AwaitResponse");
  auto &client_awaitresponse_transitions =
      client_awaitresponse_state->transitions;
  ASSERT_EQ(1, client_awaitresponse_transitions.size());
  auto &client_receive_response_transition_pair =
      client_awaitresponse_transitions.at("HTTP Response");
  ASSERT_EQ(client_receive_response_transition_pair.second, "Open");
  auto &client_receive_response_transition =
      std::get<std::shared_ptr<const sema::ast::ReadTransition>>(
          client_receive_response_transition_pair.first);
  ASSERT_EQ(6, client_receive_response_transition->actions.size());
  const auto &client_receive_response_actions =
      client_receive_response_transition->actions;
  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::ReadStaticOctets>>(
      client_receive_response_actions[0]));
  auto read_static_octets_0 =
      std::get<std::shared_ptr<const sema::ast::action::ReadStaticOctets>>(
          client_receive_response_actions[0]);
  ASSERT_EQ(read_static_octets_0->octets, "HTTP/");

  ASSERT_TRUE(
      std::holds_alternative<
          std::shared_ptr<const sema::ast::action::ReadOctetsUntilTerminator>>(
          client_receive_response_actions[1]));
  auto read_octets_until_terminator_2 = std::get<
      std::shared_ptr<const sema::ast::action::ReadOctetsUntilTerminator>>(
      client_receive_response_actions[1]);
  ASSERT_EQ(read_octets_until_terminator_2->terminator, ".");
  ASSERT_EQ(read_octets_until_terminator_2->identifier->name, "major_version");

  ASSERT_TRUE(
      std::holds_alternative<
          std::shared_ptr<const sema::ast::action::ReadOctetsUntilTerminator>>(
          client_receive_response_actions[2]));

  ASSERT_TRUE(
      std::holds_alternative<
          std::shared_ptr<const sema::ast::action::ReadOctetsUntilTerminator>>(
          client_receive_response_actions[3]));

  ASSERT_TRUE(
      std::holds_alternative<
          std::shared_ptr<const sema::ast::action::ReadOctetsUntilTerminator>>(
          client_receive_response_actions[4]));

  ASSERT_TRUE(
      std::holds_alternative<std::shared_ptr<const sema::ast::action::Loop>>(
          client_receive_response_actions[5]));

  auto &server_awaitresponse_state = server_states.at("AwaitResponse");
  auto &server_awaitresponse_transitions =
      server_awaitresponse_state->transitions;
  ASSERT_EQ(1, server_awaitresponse_transitions.size());
  auto &server_send_response_transition_pair =
      server_awaitresponse_transitions.at("HTTP Response");
  ASSERT_EQ(server_send_response_transition_pair.second, "Open");
  auto &server_send_response_transition =
      std::get<std::shared_ptr<const sema::ast::WriteTransition>>(
          server_send_response_transition_pair.first);
  ASSERT_EQ(10, server_send_response_transition->actions.size());
  const auto &server_send_response_actions =
      server_send_response_transition->actions;
  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
      server_send_response_actions[0]));
  auto write_static_octets_0 =
      std::get<std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
          server_send_response_actions[0]);
  ASSERT_EQ(write_static_octets_0->octets, "HTTP/");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
      server_send_response_actions[1]));
  auto write_from_identifier_1 =
      std::get<std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
          server_send_response_actions[1]);
  ASSERT_EQ(write_from_identifier_1->identifier->name, "major_version");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
      server_send_response_actions[2]));
  auto write_static_octets_2 =
      std::get<std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
          server_send_response_actions[2]);
  ASSERT_EQ(write_static_octets_2->octets, ".");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
      server_send_response_actions[3]));
  auto write_from_identifier_3 =
      std::get<std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
          server_send_response_actions[3]);
  ASSERT_EQ(write_from_identifier_3->identifier->name, "minor_version");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
      server_send_response_actions[4]));
  auto write_static_octets_4 =
      std::get<std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
          server_send_response_actions[4]);
  ASSERT_EQ(write_static_octets_4->octets, " ");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
      server_send_response_actions[5]));
  auto write_from_identifier_server_5 =
      std::get<std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
          server_send_response_actions[5]);
  ASSERT_EQ(write_from_identifier_server_5->identifier->name, "response_code");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
      server_send_response_actions[6]));
  auto server_write_static_octets_6 =
      std::get<std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
          server_send_response_actions[6]);
  ASSERT_EQ(server_write_static_octets_6->octets, " ");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
      server_send_response_actions[7]));
  auto server_write_from_identifier_7 =
      std::get<std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
          server_send_response_actions[7]);
  ASSERT_EQ(server_write_from_identifier_7->identifier->name, "reason_phrase");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
      server_send_response_actions[8]));
  auto server_write_static_octets_8 =
      std::get<std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
          server_send_response_actions[8]);
  ASSERT_EQ(server_write_static_octets_8->octets, "\r\n");

  ASSERT_TRUE(
      std::holds_alternative<std::shared_ptr<const sema::ast::action::Loop>>(
          server_send_response_actions[9]));
  auto server_loop_9 = std::get<std::shared_ptr<const sema::ast::action::Loop>>(
      server_send_response_actions[9]);
  ASSERT_EQ(server_loop_9->variable->name, "header");
  ASSERT_EQ(server_loop_9->collection->name, "headers");
  ASSERT_EQ(server_loop_9->terminator, "\r\n");
  ASSERT_EQ(server_loop_9->actions.size(), 4);
  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
      server_loop_9->actions[0]));
  auto server_write_loop_action_0 =
      std::get<std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
          server_loop_9->actions[0]);
  ASSERT_EQ(server_write_loop_action_0->identifier->name, "header");
  ASSERT_TRUE(server_write_loop_action_0->identifier->member.has_value());
  ASSERT_EQ(server_write_loop_action_0->identifier->member.value()->name,
            "key");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
      server_loop_9->actions[1]));
  auto server_write_loop_action_1 =
      std::get<std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
          server_loop_9->actions[1]);
  ASSERT_EQ(server_write_loop_action_1->octets, ":");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
      server_loop_9->actions[2]));
  auto server_write_loop_action_2 =
      std::get<std::shared_ptr<const sema::ast::action::WriteFromIdentifier>>(
          server_loop_9->actions[2]);
  ASSERT_EQ(server_write_loop_action_2->identifier->name, "header");
  ASSERT_TRUE(server_write_loop_action_2->identifier->member.has_value());
  ASSERT_EQ(server_write_loop_action_2->identifier->member.value()->name,
            "value");

  ASSERT_TRUE(std::holds_alternative<
              std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
      server_loop_9->actions[3]));
  auto server_write_loop_action_3 =
      std::get<std::shared_ptr<const sema::ast::action::WriteStaticOctets>>(
          server_loop_9->actions[3]);
  ASSERT_EQ(server_write_loop_action_3->octets, "\r\n");
}
