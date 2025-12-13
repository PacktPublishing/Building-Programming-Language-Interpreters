/**
 * Test protocol mismatch handling with the generated server using GeneratedServerWrapper.
 *
 * This test verifies that the server correctly handles protocol mismatches:
 * 1. Sending invalid/unexpected commands
 * 2. Sending garbage data
 * 3. Server closes connection on protocol error
 *
 * The server should detect the mismatch and close the connection gracefully.
 * 
 * This test uses GeneratedServerWrapper from networkprotocoldsl_uv to ensure
 * the same code path as the real example server is exercised.
 */
#include "protocol.hpp"

#include <networkprotocoldsl_uv/asyncworkqueue.hpp>
#include <networkprotocoldsl_uv/generatedserverwrapper.hpp>

#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>
#include <atomic>
#include <future>
#include <variant>

#include <uv.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace smtp::generated;
using namespace networkprotocoldsl_uv;

// Test state tracking
std::atomic<int> tests_completed{0};
std::atomic<int> connections_closed_by_server{0};

constexpr int NUM_TESTS = 3;

/**
 * Handler for protocol mismatch test.
 */
struct MismatchHandler {
    MismatchHandler() = default;
    
    OpenOutput on_Open() const {
        SMTPServerGreetingData greeting;
        greeting.code_tens = 22;
        greeting.msg = "Test SMTP Server Ready";
        return greeting;
    }
    
    AwaitServerEHLOResponseOutput on_AwaitServerEHLOResponse(const SMTPEHLOCommandData& msg) const {
        SMTPEHLOSuccessResponseData response;
        response.client_domain = msg.client_domain;
        response.code_tens = 50;
        response.msg = "Hello";
        return response;
    }
    
    AwaitServerMAILFROMResponseOutput on_AwaitServerMAILFROMResponse(const SMTPMAILFROMCommandData& msg) const {
        SMTPMAILFROMSuccessResponseData response;
        response.client_domain = msg.client_domain;
        response.sender = msg.sender;
        response.code_tens = 50;
        response.msg = "Sender OK";
        return response;
    }
    
    AwaitServerRCPTTOResponseOutput on_AwaitServerRCPTTOResponse(const SMTPRCPTTOCommandData& msg) const {
        SMTPRCPTTOSuccessResponseData response;
        response.client_domain = msg.client_domain;
        response.sender = msg.sender;
        response.code_tens = 50;
        response.msg = "Recipient OK";
        return response;
    }
    
    AwaitServerRCPTTOResponseOutput on_AwaitServerRCPTTOResponse(const AdditionalSMTPRCPTTOCommandData& msg) const {
        SMTPRCPTTOSuccessResponseData response;
        response.client_domain = msg.client_domain;
        response.sender = msg.sender;
        response.code_tens = 50;
        response.msg = "Recipient OK";
        return response;
    }
    
    AwaitServerDATAResponseOutput on_AwaitServerDATAResponse(const SMTPDATACommandData& msg) const {
        SMTPDATAResponseData response;
        response.client_domain = msg.client_domain;
        response.sender = msg.sender;
        response.code_tens = 54;
        response.msg = "Start mail input";
        return response;
    }
    
    AwaitServerDATAContentResponseOutput on_AwaitServerDATAContentResponse(const SMTPDATAContentData& msg) const {
        SMTPDATAWrittenData written;
        written.client_domain = msg.client_domain;
        written.code_tens = 50;
        written.msg = "Message accepted";
        return written;
    }
    
    AwaitServerQUITResponseOutput on_AwaitServerQUITResponse(const SMTPQUITCommandData& msg) const {
        SMTPQUITResponseData response;
        response.code_tens = 21;
        response.msg = "Goodbye";
        return response;
    }
    
    AwaitServerQUITResponseOutput on_AwaitServerQUITResponse(const SMTPQUITCommandFromEHLOData& msg) const {
        SMTPQUITResponseData response;
        response.code_tens = 21;
        response.msg = "Goodbye";
        return response;
    }
    
    AwaitServerQUITResponseOutput on_AwaitServerQUITResponse(const SMTPQUITCommandFromFirstRCPTTOData& msg) const {
        SMTPQUITResponseData response;
        response.code_tens = 21;
        response.msg = "Goodbye";
        return response;
    }
    
    AwaitServerQUITResponseOutput on_AwaitServerQUITResponse(const SMTPQUITCommandFromRCPTTOOrDATAData& msg) const {
        SMTPQUITResponseData response;
        response.code_tens = 21;
        response.msg = "Goodbye";
        return response;
    }
};

// Test 1: Send garbage data immediately after greeting
bool test_garbage_data(int port) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Test 1: Failed to create socket" << std::endl;
        return false;
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    
    bool success = false;
    
    if (connect(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0) {
        std::cout << "Test 1: Connected, sending garbage data" << std::endl;
        
        char buf[1024];
        
        // Read greeting
        ssize_t n = recv(sock, buf, sizeof(buf), 0);
        if (n > 0) {
            std::cout << "Test 1: Received greeting" << std::endl;
        }
        
        // Send garbage data (not a valid SMTP command)
        const char* garbage = "\x01\x02\x03\xFF\xFE\xFD GARBAGE DATA\r\n";
        send(sock, garbage, strlen(garbage), 0);
        std::cout << "Test 1: Sent garbage data" << std::endl;
        
        // Wait a bit for server to process
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Try to read - should get connection closed or error
        n = recv(sock, buf, sizeof(buf), 0);
        if (n <= 0) {
            std::cout << "Test 1: Connection closed by server (expected)" << std::endl;
            connections_closed_by_server.fetch_add(1);
            success = true;
        } else {
            std::cout << "Test 1: Unexpected data received: " << std::string(buf, n) << std::endl;
        }
    } else {
        std::cerr << "Test 1: Failed to connect" << std::endl;
    }
    
    close(sock);
    tests_completed.fetch_add(1);
    return success;
}

// Test 2: Send invalid command after EHLO
bool test_invalid_command(int port) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Test 2: Failed to create socket" << std::endl;
        return false;
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    
    bool success = false;
    
    if (connect(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0) {
        std::cout << "Test 2: Connected, will send invalid command" << std::endl;
        
        char buf[1024];
        ssize_t n;
        
        // Read greeting
        n = recv(sock, buf, sizeof(buf), 0);
        
        // Send valid EHLO
        const char* ehlo = "EHLO test.example.com\r\n";
        send(sock, ehlo, strlen(ehlo), 0);
        
        // Read EHLO response
        n = recv(sock, buf, sizeof(buf), 0);
        
        // Now send an invalid command (not QUIT or MAIL FROM which are expected)
        const char* invalid = "INVALID COMMAND HERE\r\n";
        send(sock, invalid, strlen(invalid), 0);
        std::cout << "Test 2: Sent invalid command after EHLO" << std::endl;
        
        // Wait a bit for server to process
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Try to read - should get connection closed or error
        n = recv(sock, buf, sizeof(buf), 0);
        if (n <= 0) {
            std::cout << "Test 2: Connection closed by server (expected)" << std::endl;
            connections_closed_by_server.fetch_add(1);
            success = true;
        } else {
            std::cout << "Test 2: Unexpected data received: " << std::string(buf, n) << std::endl;
        }
    } else {
        std::cerr << "Test 2: Failed to connect" << std::endl;
    }
    
    close(sock);
    tests_completed.fetch_add(1);
    return success;
}

// Test 3: Send command out of sequence
bool test_out_of_sequence(int port) {
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Test 3: Failed to create socket" << std::endl;
        return false;
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    
    bool success = false;
    
    if (connect(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0) {
        std::cout << "Test 3: Connected, will send command out of sequence" << std::endl;
        
        char buf[1024];
        ssize_t n;
        
        // Read greeting
        n = recv(sock, buf, sizeof(buf), 0);
        
        // Send DATA command directly (without EHLO, MAIL FROM, RCPT TO)
        // The server expects EHLO or QUIT at this point
        const char* data = "DATA\r\n";
        send(sock, data, strlen(data), 0);
        std::cout << "Test 3: Sent DATA without prior EHLO/MAIL FROM/RCPT TO" << std::endl;
        
        // Wait a bit for server to process
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Try to read - should get connection closed or error
        n = recv(sock, buf, sizeof(buf), 0);
        if (n <= 0) {
            std::cout << "Test 3: Connection closed by server (expected)" << std::endl;
            connections_closed_by_server.fetch_add(1);
            success = true;
        } else {
            std::cout << "Test 3: Unexpected data received: " << std::string(buf, n) << std::endl;
        }
    } else {
        std::cerr << "Test 3: Failed to connect" << std::endl;
    }
    
    close(sock);
    tests_completed.fetch_add(1);
    return success;
}

int main() {
    std::cout << "=== Testing Protocol Mismatch Handling (using GeneratedServerWrapper) ===" << std::endl;
    
    // Create libuv loop and async queue
    uv_loop_t* loop = uv_default_loop();
    AsyncWorkQueue async_queue(loop);
    
    // Create the shared handler
    MismatchHandler handler;
    
    // Create server wrapper using the generated runner - same as the example server
    using Runner = ServerRunner<MismatchHandler>;
    GeneratedServerWrapper<Runner, MismatchHandler> server(handler, async_queue);
    
    // Start the server on a random port
    auto bind_future = server.start("127.0.0.1", 0);
    
    // Run the event loop in a separate thread using UV_RUN_DEFAULT
    // This is how the real example server works
    std::thread io_thread([loop]() {
        uv_run(loop, UV_RUN_DEFAULT);
    });
    
    // Get the bound port
    auto bind_result = bind_future.get();
    if (std::holds_alternative<std::string>(bind_result)) {
        std::cerr << "Failed to bind: " << std::get<std::string>(bind_result) << std::endl;
        uv_stop(loop);
        io_thread.join();
        return 1;
    }
    
    int fd = std::get<int>(bind_result);
    
    // Get the actual port from the socket
    struct sockaddr_storage storage;
    socklen_t len = sizeof(storage);
    getsockname(fd, reinterpret_cast<struct sockaddr*>(&storage), &len);
    int port = ntohs(reinterpret_cast<struct sockaddr_in*>(&storage)->sin_port);
    
    std::cout << "Server listening on port " << port << std::endl;
    
    // Start test threads
    std::thread test1(test_garbage_data, port);
    std::thread test2(test_invalid_command, port);
    std::thread test3(test_out_of_sequence, port);
    
    // Wait for all tests to complete (with timeout)
    auto start_time = std::chrono::steady_clock::now();
    while (tests_completed.load() < NUM_TESTS) {
        auto elapsed = std::chrono::steady_clock::now() - start_time;
        if (elapsed > std::chrono::seconds(10)) {
            std::cout << "Test timed out!" << std::endl;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    test1.join();
    test2.join();
    test3.join();
    
    // Stop the server - this will block until server handle is closed
    std::cout << "Stopping server..." << std::endl;
    server.stop();
    
    // Shutdown the async queue - this closes the async handle and lets the loop exit
    async_queue.shutdown().wait();
    io_thread.join();
    
    std::cout << "Tests completed: " << tests_completed.load() << "/" << NUM_TESTS << std::endl;
    std::cout << "Connections closed by server: " << connections_closed_by_server.load() << "/" << NUM_TESTS << std::endl;
    
    // All 3 tests should have triggered server-side connection closes
    if (connections_closed_by_server.load() == NUM_TESTS) {
        std::cout << "=== PROTOCOL_MISMATCH_TEST_SUCCESS ===" << std::endl;
        return 0;
    } else {
        std::cout << "=== PROTOCOL_MISMATCH_TEST_FAILED ===" << std::endl;
        return 1;
    }
}
