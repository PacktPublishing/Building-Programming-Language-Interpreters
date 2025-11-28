/**
 * Test for GeneratedServerWrapper with generated SMTP protocol
 *
 * This test verifies that the libuv adapter works correctly with
 * generated protocol code by:
 * 1. Starting a server with GeneratedServerWrapper
 * 2. Connecting a client
 * 3. Performing an EHLO/QUIT exchange
 * 4. Verifying proper cleanup
 */
#include "protocol.hpp"
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

// Global state for the test
std::atomic<bool> test_passed{false};
std::promise<void> test_done_promise;

/**
 * Handler that satisfies ServerHandlerConcept.
 * 
 * IMPORTANT: This handler is shared across ALL connections. It must NOT store
 * any per-connection state as member variables. All connection-specific data
 * flows through the message input/output parameters.
 *
 * ALL METHODS ARE CONST to enforce this constraint at compile-time.
 *
 * Methods are named on_<StateName> with overloads for each message type
 * that can arrive at that state. The runner uses std::visit + ADL to
 * dispatch to the correct overload.
 */
struct SMTPServerHandler {
    // NOTE: No per-connection state here! Only shared configuration.
    
    SMTPServerHandler() {
        std::cout << "Handler created (shared across all connections)" << std::endl;
    }
    
    // Handle Open state - send server greeting (no input needed)
    OpenOutput on_Open() const {
        std::cout << "Server: Sending greeting" << std::endl;
        SMTPServerGreetingData greeting;
        greeting.code_tens = 50;
        greeting.msg = "Test SMTP Server Ready";
        return greeting;
    }
    
    // AwaitServerEHLOResponse state - receives EHLO command
    AwaitServerEHLOResponseOutput on_AwaitServerEHLOResponse(const SMTPEHLOCommandData& msg) const {
        std::cout << "Server: Received EHLO from " << msg.client_domain << std::endl;
        SMTPEHLOSuccessResponseData response;
        response.client_domain = msg.client_domain;
        response.code_tens = 50;
        response.msg = "Hello, pleased to meet you";
        return response;
    }
    
    // AwaitServerMAILFROMResponse state - receives MAIL FROM command
    AwaitServerMAILFROMResponseOutput on_AwaitServerMAILFROMResponse(const SMTPMAILFROMCommandData& msg) const {
        std::cout << "Server: Received MAIL FROM " << msg.sender << std::endl;
        SMTPMAILFROMSuccessResponseData response;
        response.client_domain = msg.client_domain;
        response.sender = msg.sender;
        response.code_tens = 50;
        response.msg = "Sender OK";
        return response;
    }
    
    // AwaitServerRCPTTOResponse state - receives first RCPT TO or additional RCPT TO
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
    
    // AwaitServerDATAResponse state - receives DATA command
    AwaitServerDATAResponseOutput on_AwaitServerDATAResponse(const SMTPDATACommandData& msg) const {
        SMTPDATAResponseData response;
        response.client_domain = msg.client_domain;
        response.sender = msg.sender;
        response.code_tens = 54;
        response.msg = "Start mail input";
        return response;
    }
    
    // AwaitServerDATAContentResponse state - receives DATA content
    AwaitServerDATAContentResponseOutput on_AwaitServerDATAContentResponse(const SMTPDATAContentData& msg) const {
        SMTPDATAWrittenData written;
        written.client_domain = msg.client_domain;
        written.code_tens = 50;
        written.msg = "Message accepted";
        return written;
    }
    
    // AwaitServerQUITResponse state - receives various QUIT commands
    AwaitServerQUITResponseOutput on_AwaitServerQUITResponse(const SMTPQUITCommandData& msg) const {
        std::cout << "Server: Received QUIT" << std::endl;
        test_passed.store(true);
        SMTPQUITResponseData response;
        response.code_tens = 21;
        response.msg = "Goodbye";
        return response;
    }
    
    AwaitServerQUITResponseOutput on_AwaitServerQUITResponse(const SMTPQUITCommandFromEHLOData& msg) const {
        std::cout << "Server: Received QUIT from EHLO state" << std::endl;
        test_passed.store(true);
        SMTPQUITResponseData response;
        response.code_tens = 21;
        response.msg = "Goodbye";
        return response;
    }
    
    AwaitServerQUITResponseOutput on_AwaitServerQUITResponse(const SMTPQUITCommandFromFirstRCPTTOData& msg) const {
        std::cout << "Server: Received QUIT from first RCPTTO state" << std::endl;
        test_passed.store(true);
        SMTPQUITResponseData response;
        response.code_tens = 21;
        response.msg = "Goodbye";
        return response;
    }
    
    AwaitServerQUITResponseOutput on_AwaitServerQUITResponse(const SMTPQUITCommandFromRCPTTOOrDATAData& msg) const {
        std::cout << "Server: Received QUIT from RCPTTO/DATA state" << std::endl;
        test_passed.store(true);
        SMTPQUITResponseData response;
        response.code_tens = 21;
        response.msg = "Goodbye";
        return response;
    }
};

// Server state
struct ServerState {
    uv_tcp_t server;
    uv_loop_t* loop;
    const SMTPServerHandler handler;  // Shared handler for all connections (const!)
    std::vector<std::unique_ptr<ServerRunner<SMTPServerHandler>>> runners;
    std::vector<uv_tcp_t*> handles;
};

void on_close(uv_handle_t* handle) {
    delete reinterpret_cast<uv_tcp_t*>(handle);
}

struct WriteRequest {
    uv_write_t req;
    char* buffer;
};

void on_write(uv_write_t* req, int status) {
    auto* write_req = reinterpret_cast<WriteRequest*>(req);
    delete[] write_req->buffer;
    delete write_req;
}

template<typename Runner>
void process_output(Runner& runner, uv_tcp_t* client) {
    while (runner.has_pending_output()) {
        std::string_view out = runner.pending_output();
        char* buf = new char[out.size()];
        std::memcpy(buf, out.data(), out.size());
        runner.bytes_written(out.size());
        
        auto* write_req = new WriteRequest;
        write_req->buffer = buf;
        uv_buf_t uvbuf = uv_buf_init(buf, out.size());
        uv_write(&write_req->req, reinterpret_cast<uv_stream_t*>(client), &uvbuf, 1, on_write);
    }
}

void on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    auto* runner = static_cast<ServerRunner<SMTPServerHandler>*>(stream->data);
    
    if (nread > 0) {
        runner->on_bytes_received(std::string_view(buf->base, nread));
        process_output(*runner, reinterpret_cast<uv_tcp_t*>(stream));
        
        if (runner->is_closed()) {
            std::cout << "Server: Connection closed, signaling done" << std::endl;
            uv_close(reinterpret_cast<uv_handle_t*>(stream), on_close);
            uv_stop(uv_handle_get_loop(reinterpret_cast<uv_handle_t*>(stream)));
            test_done_promise.set_value();
        }
    } else if (nread < 0) {
        uv_close(reinterpret_cast<uv_handle_t*>(stream), on_close);
    }
    
    if (buf->base) free(buf->base);
}

void alloc_buffer(uv_handle_t* handle, size_t suggested, uv_buf_t* buf) {
    buf->base = static_cast<char*>(malloc(suggested));
    buf->len = suggested;
}

void on_connection(uv_stream_t* server, int status) {
    if (status < 0) return;
    
    auto* state = static_cast<ServerState*>(server->data);
    
    uv_tcp_t* client = new uv_tcp_t;
    uv_tcp_init(state->loop, client);
    
    if (uv_accept(server, reinterpret_cast<uv_stream_t*>(client)) == 0) {
        // Each connection gets its own Runner, but shares the Handler
        auto runner = std::make_unique<ServerRunner<SMTPServerHandler>>(state->handler);
        
        // Call start() to trigger on_Open and send the greeting
        runner->start();
        process_output(*runner, client);
        
        client->data = runner.get();
        state->runners.push_back(std::move(runner));
        state->handles.push_back(client);
        
        uv_read_start(reinterpret_cast<uv_stream_t*>(client), alloc_buffer, on_read);
    } else {
        uv_close(reinterpret_cast<uv_handle_t*>(client), on_close);
    }
}

int main() {
    std::cout << "=== Testing Concept-based ServerRunner with SMTP ===" << std::endl;
    
    uv_loop_t* loop = uv_default_loop();
    
    ServerState state;
    state.loop = loop;
    
    // Initialize server
    uv_tcp_init(loop, &state.server);
    state.server.data = &state;
    
    struct sockaddr_in addr;
    uv_ip4_addr("127.0.0.1", 0, &addr);  // Port 0 = let OS choose
    
    uv_tcp_bind(&state.server, reinterpret_cast<const struct sockaddr*>(&addr), 0);
    uv_listen(reinterpret_cast<uv_stream_t*>(&state.server), 1, on_connection);
    
    // Get the actual port
    struct sockaddr_storage storage;
    int namelen = sizeof(storage);
    uv_tcp_getsockname(&state.server, reinterpret_cast<struct sockaddr*>(&storage), &namelen);
    int port = ntohs(reinterpret_cast<struct sockaddr_in*>(&storage)->sin_port);
    
    std::cout << "Server listening on port " << port << std::endl;
    
    // Start client in separate thread
    std::thread client_thread([port]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
        
        if (connect(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0) {
            std::cout << "Client connected" << std::endl;
            
            // Read greeting
            char buf[1024];
            ssize_t n = recv(sock, buf, sizeof(buf), 0);
            if (n > 0) {
                std::cout << "Client received: " << std::string(buf, n);
            }
            
            // Send EHLO
            const char* ehlo = "EHLO test.example.com\r\n";
            send(sock, ehlo, strlen(ehlo), 0);
            std::cout << "Client sent EHLO" << std::endl;
            
            // Read EHLO response
            n = recv(sock, buf, sizeof(buf), 0);
            if (n > 0) {
                std::cout << "Client received: " << std::string(buf, n);
            }
            
            // Send QUIT
            const char* quit = "QUIT\r\n";
            send(sock, quit, strlen(quit), 0);
            std::cout << "Client sent QUIT" << std::endl;
            
            // Read QUIT response
            n = recv(sock, buf, sizeof(buf), 0);
            if (n > 0) {
                std::cout << "Client received: " << std::string(buf, n);
            }
        }
        
        close(sock);
    });
    
    // Run loop with timeout
    auto test_done = test_done_promise.get_future();
    std::thread timer_thread([loop, &test_done]() {
        if (test_done.wait_for(std::chrono::seconds(5)) == std::future_status::timeout) {
            std::cout << "Test timed out!" << std::endl;
            uv_stop(loop);
        }
    });
    
    uv_run(loop, UV_RUN_DEFAULT);
    
    client_thread.join();
    timer_thread.join();
    
    // Cleanup
    uv_close(reinterpret_cast<uv_handle_t*>(&state.server), nullptr);
    uv_run(loop, UV_RUN_NOWAIT);
    
    if (test_passed.load()) {
        std::cout << "=== UV_ADAPTER_TEST_SUCCESS ===" << std::endl;
        return 0;
    } else {
        std::cout << "=== UV_ADAPTER_TEST_FAILED ===" << std::endl;
        return 1;
    }
}
