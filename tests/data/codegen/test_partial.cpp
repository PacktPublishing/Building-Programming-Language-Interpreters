#include "protocol.hpp"
#include <iostream>

int main() {
    smtp::generated::ServerStateMachine server;
    
    // Send greeting to set up state
    smtp::generated::SMTPServerGreetingData greeting;
    greeting.code_tens = 50;
    greeting.msg = "Ready";
    server.send_SMTPServerGreeting(greeting);
    server.bytes_written(server.pending_output().size());
    
    std::cout << "Initial state set" << std::endl;
    
    // Feed partial EHLO command in chunks
    std::string full_cmd = "EHLO test.domain.com\r\n";
    
    // Feed first part: "EHLO "
    std::string part1 = full_cmd.substr(0, 5);
    size_t consumed1 = server.on_bytes_received(part1);
    std::cout << "Part1 consumed: " << consumed1 << ", has_message: " 
              << (server.has_message() ? "yes" : "no") << std::endl;
    
    // Sans-IO property: should return without blocking even with incomplete data
    // (consumed might be 0 or partial, has_message should be false)
    
    // Feed second part: "test.domain"
    std::string part2 = full_cmd.substr(5, 11);
    size_t consumed2 = server.on_bytes_received(part2);
    std::cout << "Part2 consumed: " << consumed2 << ", has_message: " 
              << (server.has_message() ? "yes" : "no") << std::endl;
    
    // Still incomplete - no terminator yet
    
    // Feed final part with terminator: ".com\r\n"
    std::string part3 = full_cmd.substr(16);
    size_t consumed3 = server.on_bytes_received(part3);
    std::cout << "Part3 consumed: " << consumed3 << ", has_message: " 
              << (server.has_message() ? "yes" : "no") << std::endl;
    
    // Now the message should be complete
    if (server.has_message()) {
        auto msg = server.take_AwaitServerEHLOResponse_message();
        if (auto* data = std::get_if<smtp::generated::SMTPEHLOCommandData>(&msg)) {
            std::cout << "PARSED_DOMAIN:" << data->client_domain << std::endl;
        }
        std::cout << "SANS_IO_SUCCESS" << std::endl;
    } else {
        std::cout << "SANS_IO_FAILED" << std::endl;
    }
    
    return 0;
}
