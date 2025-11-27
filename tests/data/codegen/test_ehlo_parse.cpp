#include "protocol.hpp"
#include "parser.hpp"
#include "states.hpp"
#include <iostream>
#include <cstring>

int main() {
    // First, verify the parsers work in isolation
    std::string ehlo_cmd = "EHLO example.com\r\n";
    std::string_view ehlo_view(ehlo_cmd);
    
    smtp::generated::SMTPQUITCommandFromEHLOParser quit_parser;
    quit_parser.reset();
    auto quit_result = quit_parser.parse(ehlo_view);
    std::cout << "QUIT_PARSER_STATUS:" << static_cast<int>(quit_result.status) << std::endl;
    std::cout << "QUIT_PARSER_CONSUMED:" << quit_result.consumed << std::endl;
    
    smtp::generated::SMTPEHLOCommandParser ehlo_parser;
    ehlo_parser.reset();
    auto ehlo_result = ehlo_parser.parse(ehlo_view);
    std::cout << "EHLO_PARSER_STATUS:" << static_cast<int>(ehlo_result.status) << std::endl;
    std::cout << "EHLO_PARSER_CONSUMED:" << ehlo_result.consumed << std::endl;
    
    if (ehlo_parser.is_complete()) {
        auto data = ehlo_parser.take_data();
        std::cout << "PARSER_DOMAIN:" << data.client_domain << std::endl;
    }
    
    // Print state enum values for debugging
    std::cout << "STATE_OPEN:" << static_cast<int>(smtp::generated::State::Open) << std::endl;
    std::cout << "STATE_CLIENT_SEND_EHLO:" << static_cast<int>(smtp::generated::State::ClientSendEHLO) << std::endl;
    std::cout << "STATE_AWAIT_EHLO_RESPONSE:" << static_cast<int>(smtp::generated::State::AwaitServerEHLOResponse) << std::endl;
    
    // Now test through state machine
    smtp::generated::ServerStateMachine sm;
    
    std::cout << "INITIAL_STATE:" << static_cast<int>(sm.current_state()) << std::endl;
    
    // First, send a greeting to move to ClientSendEHLO state
    smtp::generated::SMTPServerGreetingData greeting;
    greeting.code_tens = 50;
    greeting.msg = "Ready";
    sm.send_SMTPServerGreeting(greeting);
    
    std::cout << "PENDING_OUTPUT:" << sm.pending_output().size() << std::endl;
    sm.bytes_written(sm.pending_output().size());
    
    // Now we should be in ClientSendEHLO state
    std::cout << "STATE_BEFORE:" << static_cast<int>(sm.current_state()) << std::endl;
    std::cout << "IS_CLOSED:" << (sm.is_closed() ? "yes" : "no") << std::endl;
    std::cout << "INPUT_SIZE:" << ehlo_cmd.size() << std::endl;
    std::cout << "INPUT_EMPTY:" << (ehlo_cmd.empty() ? "yes" : "no") << std::endl;
    
    // Verify the state value matches what we expect
    bool state_matches = (sm.current_state() == smtp::generated::State::ClientSendEHLO);
    std::cout << "STATE_MATCHES_EHLO:" << (state_matches ? "yes" : "no") << std::endl;
    
    // Parse an EHLO command using string_view
    std::string_view input_view(ehlo_cmd);
    size_t consumed = sm.on_bytes_received(input_view);
    
    std::cout << "CONSUMED:" << consumed << std::endl;
    std::cout << "HAS_MESSAGE:" << (sm.has_message() ? "yes" : "no") << std::endl;
    std::cout << "STATE_AFTER:" << static_cast<int>(sm.current_state()) << std::endl;
    
    if (sm.has_message()) {
        std::cout << "MESSAGE_STATE:" << static_cast<int>(sm.message_state()) << std::endl;
        auto msg = sm.take_AwaitServerEHLOResponse_message();
        // The message should be SMTPEHLOCommandData
        if (auto* data = std::get_if<smtp::generated::SMTPEHLOCommandData>(&msg)) {
            std::cout << "DOMAIN:" << data->client_domain << std::endl;
        } else {
            std::cout << "WRONG_VARIANT_TYPE" << std::endl;
        }
    }
    
    return 0;
}
