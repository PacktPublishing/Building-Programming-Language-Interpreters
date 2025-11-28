#include "protocol.hpp"
#include <iostream>

int main() {
    smtp::generated::ServerStateMachine server;
    smtp::generated::ClientStateMachine client;
    
    // === Server sends greeting ===
    smtp::generated::SMTPServerGreetingData greeting;
    greeting.code_tens = 50;
    greeting.msg = "Welcome to test server";
    server.send_SMTPServerGreeting(greeting);
    
    std::string greeting_bytes(server.pending_output());
    server.bytes_written(greeting_bytes.size());
    
    std::cout << "SERVER_SENT:" << greeting_bytes.size() << " bytes" << std::endl;
    
    // === Client receives greeting ===
    size_t consumed = client.on_bytes_received(greeting_bytes);
    std::cout << "CLIENT_CONSUMED:" << consumed << std::endl;
    std::cout << "CLIENT_HAS_MESSAGE:" << (client.has_message() ? "yes" : "no") << std::endl;
    
    if (client.has_message()) {
        auto msg = client.take_ClientSendEHLO_message();
        if (auto* data = std::get_if<smtp::generated::SMTPServerGreetingData>(&msg)) {
            std::cout << "GREETING_CODE:" << static_cast<int>(data->code_tens) << std::endl;
            std::cout << "GREETING_MSG:" << data->msg << std::endl;
        }
    }
    
    // === Client sends EHLO ===
    smtp::generated::SMTPEHLOCommandData ehlo;
    ehlo.client_domain = "test.example.org";
    client.send_SMTPEHLOCommand(ehlo);
    
    std::string ehlo_bytes(client.pending_output());
    client.bytes_written(ehlo_bytes.size());
    
    std::cout << "CLIENT_SENT:" << ehlo_bytes.size() << " bytes" << std::endl;
    std::cout << "EHLO_WIRE:" << ehlo_bytes;
    
    // === Server receives EHLO ===
    consumed = server.on_bytes_received(ehlo_bytes);
    std::cout << "SERVER_CONSUMED:" << consumed << std::endl;
    std::cout << "SERVER_HAS_MESSAGE:" << (server.has_message() ? "yes" : "no") << std::endl;
    
    if (server.has_message()) {
        auto msg = server.take_AwaitServerEHLOResponse_message();
        if (auto* data = std::get_if<smtp::generated::SMTPEHLOCommandData>(&msg)) {
            std::cout << "EHLO_DOMAIN:" << data->client_domain << std::endl;
        }
    }
    
    std::cout << "SUCCESS" << std::endl;
    return 0;
}
