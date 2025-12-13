#include "protocol.hpp"
#include <iostream>
#include <cassert>

// Helper to transfer data from one state machine to another
void transfer(smtp::generated::ServerStateMachine& from, 
              smtp::generated::ClientStateMachine& to) {
    if (from.has_pending_output()) {
        std::string data(from.pending_output());
        from.bytes_written(data.size());
        to.on_bytes_received(data);
    }
}

void transfer(smtp::generated::ClientStateMachine& from, 
              smtp::generated::ServerStateMachine& to) {
    if (from.has_pending_output()) {
        std::string data(from.pending_output());
        from.bytes_written(data.size());
        to.on_bytes_received(data);
    }
}

int main() {
    smtp::generated::ServerStateMachine server;
    smtp::generated::ClientStateMachine client;
    
    std::cout << "=== Starting SMTP conversation ===" << std::endl;
    
    // 1. Server sends greeting (Open -> ClientSendEHLO)
    {
        smtp::generated::SMTPServerGreetingData greeting;
        greeting.code_tens = 50;
        greeting.msg = "Test SMTP Server Ready";
        server.send_SMTPServerGreeting(greeting);
        std::cout << "1. Server sent greeting" << std::endl;
    }
    transfer(server, client);
    
    // Client should have received the greeting
    assert(client.has_message());
    auto greeting_msg = client.take_ClientSendEHLO_message();
    std::cout << "   Client received greeting" << std::endl;
    
    // 2. Client sends EHLO (ClientSendEHLO -> AwaitServerEHLOResponse)
    {
        smtp::generated::SMTPEHLOCommandData ehlo;
        ehlo.client_domain = "client.example.com";
        client.send_SMTPEHLOCommand(ehlo);
        std::cout << "2. Client sent EHLO" << std::endl;
    }
    transfer(client, server);
    
    // Server should have received EHLO
    assert(server.has_message());
    auto ehlo_msg = server.take_AwaitServerEHLOResponse_message();
    auto* ehlo_data = std::get_if<smtp::generated::SMTPEHLOCommandData>(&ehlo_msg);
    assert(ehlo_data != nullptr);
    std::cout << "   Server received EHLO from: " << ehlo_data->client_domain << std::endl;
    
    // 3. Server sends EHLO success response (AwaitServerEHLOResponse -> ClientSendCommand)
    {
        smtp::generated::SMTPEHLOSuccessResponseData response;
        response.client_domain = ehlo_data->client_domain;
        response.code_tens = 50;
        response.msg = "Hello client.example.com, pleased to meet you";
        server.send_SMTPEHLOSuccessResponse(response);
        std::cout << "3. Server sent EHLO success response" << std::endl;
    }
    transfer(server, client);
    
    // Client should have received response
    assert(client.has_message());
    auto ehlo_resp_msg = client.take_ClientSendCommand_message();
    std::cout << "   Client received EHLO response" << std::endl;
    
    // 4. Client sends QUIT (ClientSendCommand -> AwaitServerQUITResponse)
    {
        smtp::generated::SMTPQUITCommandData quit;
        quit.client_domain = "client.example.com";
        client.send_SMTPQUITCommand(quit);
        std::cout << "4. Client sent QUIT" << std::endl;
    }
    transfer(client, server);
    
    // Server should have received QUIT
    assert(server.has_message());
    auto quit_msg = server.take_AwaitServerQUITResponse_message();
    std::cout << "   Server received QUIT" << std::endl;
    
    // 5. Server sends QUIT response (AwaitServerQUITResponse -> Closed)
    {
        smtp::generated::SMTPQUITResponseData response;
        response.code_tens = 21;
        response.msg = "Goodbye";
        server.send_SMTPQUITResponse(response);
        std::cout << "5. Server sent QUIT response" << std::endl;
    }
    transfer(server, client);
    
    // Client should have received QUIT response and be in Closed state
    assert(client.has_message());
    auto quit_resp_msg = client.take_Closed_message();
    std::cout << "   Client received QUIT response" << std::endl;
    
    // Both should be in Closed state
    std::cout << "Server closed: " << (server.is_closed() ? "yes" : "no") << std::endl;
    std::cout << "Client closed: " << (client.is_closed() ? "yes" : "no") << std::endl;
    
    if (server.is_closed() && client.is_closed()) {
        std::cout << "=== CONVERSATION_SUCCESS ===" << std::endl;
        return 0;
    } else {
        std::cout << "=== CONVERSATION_FAILED ===" << std::endl;
        return 1;
    }
}
