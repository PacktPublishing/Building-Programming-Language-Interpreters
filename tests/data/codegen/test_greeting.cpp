#include "protocol.hpp"
#include <iostream>
#include <iomanip>

int main() {
    smtp::generated::ServerStateMachine sm;
    
    // Create a server greeting with known values
    smtp::generated::SMTPServerGreetingData greeting;
    greeting.code_tens = 50;  // This makes "250" when combined with the "2" prefix
    greeting.msg = "Hello SMTP";
    
    // Send the greeting
    sm.send_SMTPServerGreeting(greeting);
    
    // Output the serialized bytes
    auto output = sm.pending_output();
    std::cout << "OUTPUT:";
    for (char c : output) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') 
                  << (static_cast<unsigned>(c) & 0xFF) << " ";
    }
    std::cout << std::endl;
    
    // Also output as string for easy comparison
    std::cout << "STRING:" << output << std::endl;
    
    return 0;
}
