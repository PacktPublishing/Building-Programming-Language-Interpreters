/**
 * Test full SMTP email transaction with the generated server.
 *
 * This test verifies a complete email flow:
 * 1. Server sends greeting
 * 2. Client sends EHLO
 * 3. Server responds with EHLO success
 * 4. Client sends MAIL FROM
 * 5. Server responds with MAIL FROM success
 * 6. Client sends RCPT TO
 * 7. Server responds with RCPT TO success
 * 8. Client sends DATA command
 * 9. Server responds with DATA response (354)
 * 10. Client sends DATA content
 * 11. Server responds with DATA written success
 * 12. Client sends QUIT
 * 13. Server responds and closes
 *
 * The test verifies that both state machines reach the Closed state.
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

// Test state tracking
std::atomic<bool> test_passed{false};
std::atomic<int> messages_processed{0};
std::promise<void> test_done_promise;

// Expected number of messages in a complete transaction:
// EHLO, MAIL FROM, RCPT TO, DATA command, DATA content, QUIT = 6 messages
constexpr int EXPECTED_MESSAGES = 6;

/**
 * Handler for full email transaction test.
 */
struct FullTransactionHandler {
    FullTransactionHandler() {
        std::cout << "Handler created" << std::endl;
    }
    
    OpenOutput on_Open() const {
        std::cout << "Server: Sending greeting" << std::endl;
        SMTPServerGreetingData greeting;
        greeting.code_tens = 22;
        greeting.msg = "Test SMTP Server Ready";
        return greeting;
    }
    
    AwaitServerEHLOResponseOutput on_AwaitServerEHLOResponse(const SMTPEHLOCommandData& msg) const {
        std::cout << "Server: Received EHLO from " << msg.client_domain << std::endl;
        messages_processed.fetch_add(1);
        SMTPEHLOSuccessResponseData response;
        response.client_domain = msg.client_domain;
        response.code_tens = 50;
        response.msg = "Hello, pleased to meet you";
        return response;
    }
    
    AwaitServerMAILFROMResponseOutput on_AwaitServerMAILFROMResponse(const SMTPMAILFROMCommandData& msg) const {
        std::cout << "Server: Received MAIL FROM <" << msg.sender << ">" << std::endl;
        messages_processed.fetch_add(1);
        SMTPMAILFROMSuccessResponseData response;
        response.client_domain = msg.client_domain;
        response.sender = msg.sender;
        response.code_tens = 50;
        response.msg = "Sender OK";
        return response;
    }
    
    AwaitServerRCPTTOResponseOutput on_AwaitServerRCPTTOResponse(const SMTPRCPTTOCommandData& msg) const {
        std::cout << "Server: Received RCPT TO (first recipient)" << std::endl;
        messages_processed.fetch_add(1);
        SMTPRCPTTOSuccessResponseData response;
        response.client_domain = msg.client_domain;
        response.sender = msg.sender;
        response.code_tens = 50;
        response.msg = "Recipient OK";
        return response;
    }
    
    AwaitServerRCPTTOResponseOutput on_AwaitServerRCPTTOResponse(const AdditionalSMTPRCPTTOCommandData& msg) const {
        std::cout << "Server: Received RCPT TO (additional recipient)" << std::endl;
        messages_processed.fetch_add(1);
        SMTPRCPTTOSuccessResponseData response;
        response.client_domain = msg.client_domain;
        response.sender = msg.sender;
        response.code_tens = 50;
        response.msg = "Recipient OK";
        return response;
    }
    
    AwaitServerDATAResponseOutput on_AwaitServerDATAResponse(const SMTPDATACommandData& msg) const {
        std::cout << "Server: Received DATA command" << std::endl;
        messages_processed.fetch_add(1);
        SMTPDATAResponseData response;
        response.client_domain = msg.client_domain;
        response.sender = msg.sender;
        response.code_tens = 54;
        response.msg = "Start mail input; end with <CRLF>.<CRLF>";
        return response;
    }
    
    AwaitServerDATAContentResponseOutput on_AwaitServerDATAContentResponse(const SMTPDATAContentData& msg) const {
        std::cout << "Server: Received DATA content (" << msg.content.size() << " bytes)" << std::endl;
        std::cout << "Server: Content preview: " << msg.content.substr(0, 200) << "..." << std::endl;
        
        // Verify that dot-stuffing was correctly un-escaped
        // The email body should contain "\r\n.signature" (with single dot)
        // because the wire format "\r\n..signature" was un-escaped
        if (msg.content.find("\r\n.signature") != std::string::npos) {
            std::cout << "Server: Dot-stuffing correctly un-escaped!" << std::endl;
        } else {
            std::cout << "Server: WARNING - Expected to find '.signature' line" << std::endl;
        }
        
        messages_processed.fetch_add(1);
        SMTPDATAWrittenData written;
        written.client_domain = msg.client_domain;
        written.code_tens = 50;
        written.msg = "Message accepted for delivery";
        return written;
    }
    
    AwaitServerQUITResponseOutput on_AwaitServerQUITResponse(const SMTPQUITCommandData& msg) const {
        std::cout << "Server: Received QUIT" << std::endl;
        messages_processed.fetch_add(1);
        
        // Check if all expected messages were processed
        if (messages_processed.load() >= EXPECTED_MESSAGES) {
            test_passed.store(true);
        }
        
        SMTPQUITResponseData response;
        response.code_tens = 21;
        response.msg = "Goodbye";
        return response;
    }
    
    AwaitServerQUITResponseOutput on_AwaitServerQUITResponse(const SMTPQUITCommandFromEHLOData& msg) const {
        std::cout << "Server: Received QUIT from EHLO state" << std::endl;
        messages_processed.fetch_add(1);
        SMTPQUITResponseData response;
        response.code_tens = 21;
        response.msg = "Goodbye";
        return response;
    }
    
    AwaitServerQUITResponseOutput on_AwaitServerQUITResponse(const SMTPQUITCommandFromFirstRCPTTOData& msg) const {
        std::cout << "Server: Received QUIT from first RCPTTO state" << std::endl;
        messages_processed.fetch_add(1);
        SMTPQUITResponseData response;
        response.code_tens = 21;
        response.msg = "Goodbye";
        return response;
    }
    
    AwaitServerQUITResponseOutput on_AwaitServerQUITResponse(const SMTPQUITCommandFromRCPTTOOrDATAData& msg) const {
        std::cout << "Server: Received QUIT from RCPTTO/DATA state" << std::endl;
        messages_processed.fetch_add(1);
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
    const FullTransactionHandler handler;
    std::vector<std::unique_ptr<ServerRunner<FullTransactionHandler>>> runners;
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
    auto* runner = static_cast<ServerRunner<FullTransactionHandler>*>(stream->data);
    
    if (nread > 0) {
        std::cout << "Server: Received " << nread << " bytes" << std::endl;
        runner->on_bytes_received(std::string_view(buf->base, nread));
        process_output(*runner, reinterpret_cast<uv_tcp_t*>(stream));
        
        if (runner->is_closed()) {
            std::cout << "Server: Connection closed, runner is in Closed state" << std::endl;
            uv_close(reinterpret_cast<uv_handle_t*>(stream), on_close);
            uv_stop(uv_handle_get_loop(reinterpret_cast<uv_handle_t*>(stream)));
            test_done_promise.set_value();
        }
    } else if (nread < 0) {
        std::cout << "Server: Connection error or closed by client" << std::endl;
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
        std::cout << "Server: New connection accepted" << std::endl;
        
        auto runner = std::make_unique<ServerRunner<FullTransactionHandler>>(state->handler);
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
    std::cout << "=== Testing Full Email Transaction ===" << std::endl;
    
    uv_loop_t* loop = uv_default_loop();
    
    ServerState state;
    state.loop = loop;
    
    uv_tcp_init(loop, &state.server);
    state.server.data = &state;
    
    struct sockaddr_in addr;
    uv_ip4_addr("127.0.0.1", 0, &addr);
    
    uv_tcp_bind(&state.server, reinterpret_cast<const struct sockaddr*>(&addr), 0);
    uv_listen(reinterpret_cast<uv_stream_t*>(&state.server), 1, on_connection);
    
    // Get the actual port
    struct sockaddr_storage storage;
    int namelen = sizeof(storage);
    uv_tcp_getsockname(&state.server, reinterpret_cast<struct sockaddr*>(&storage), &namelen);
    int port = ntohs(reinterpret_cast<struct sockaddr_in*>(&storage)->sin_port);
    
    std::cout << "Server listening on port " << port << std::endl;
    
    // Client thread performs full email transaction
    std::thread client_thread([port]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
        
        if (connect(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0) {
            std::cout << "Client: Connected" << std::endl;
            
            char buf[4096];
            ssize_t n;
            
            // 1. Read greeting
            n = recv(sock, buf, sizeof(buf), 0);
            if (n > 0) {
                std::cout << "Client: Received greeting: " << std::string(buf, n);
            }
            
            // 2. Send EHLO
            const char* ehlo = "EHLO test.example.com\r\n";
            send(sock, ehlo, strlen(ehlo), 0);
            std::cout << "Client: Sent EHLO" << std::endl;
            
            // 3. Read EHLO response
            n = recv(sock, buf, sizeof(buf), 0);
            if (n > 0) {
                std::cout << "Client: Received: " << std::string(buf, n);
            }
            
            // 4. Send MAIL FROM
            const char* mail_from = "MAIL FROM:<sender@example.com>\r\n";
            send(sock, mail_from, strlen(mail_from), 0);
            std::cout << "Client: Sent MAIL FROM" << std::endl;
            
            // 5. Read MAIL FROM response
            n = recv(sock, buf, sizeof(buf), 0);
            if (n > 0) {
                std::cout << "Client: Received: " << std::string(buf, n);
            }
            
            // 6. Send RCPT TO
            const char* rcpt_to = "RCPT TO:<recipient@example.com>\r\n";
            send(sock, rcpt_to, strlen(rcpt_to), 0);
            std::cout << "Client: Sent RCPT TO" << std::endl;
            
            // 7. Read RCPT TO response
            n = recv(sock, buf, sizeof(buf), 0);
            if (n > 0) {
                std::cout << "Client: Received: " << std::string(buf, n);
            }
            
            // 8. Send DATA command
            const char* data_cmd = "DATA\r\n";
            send(sock, data_cmd, strlen(data_cmd), 0);
            std::cout << "Client: Sent DATA" << std::endl;
            
            // 9. Read DATA response (354)
            n = recv(sock, buf, sizeof(buf), 0);
            if (n > 0) {
                std::cout << "Client: Received: " << std::string(buf, n);
            }
            
            // 10. Send DATA content (email body + terminator)
            // Note: Lines starting with "." must be dot-stuffed (doubled) on the wire
            // The ".signature" line becomes "..signature" in the wire format
            const char* data_content = 
                "From: sender@example.com\r\n"
                "To: recipient@example.com\r\n"
                "Subject: Test Email with Dot-Stuffing\r\n"
                "\r\n"
                "This is a test email body.\r\n"
                "Here is a line with dots in the middle...no escaping needed.\r\n"
                "..signature\r\n"  // Wire format: ".signature" becomes "..signature"
                "Best regards,\r\n"
                "Test\r\n"
                ".\r\n";
            send(sock, data_content, strlen(data_content), 0);
            std::cout << "Client: Sent DATA content with dot-stuffed line" << std::endl;
            
            // 11. Read DATA written response
            n = recv(sock, buf, sizeof(buf), 0);
            if (n > 0) {
                std::cout << "Client: Received: " << std::string(buf, n);
            }
            
            // 12. Send QUIT
            const char* quit = "QUIT\r\n";
            send(sock, quit, strlen(quit), 0);
            std::cout << "Client: Sent QUIT" << std::endl;
            
            // 13. Read QUIT response
            n = recv(sock, buf, sizeof(buf), 0);
            if (n > 0) {
                std::cout << "Client: Received: " << std::string(buf, n);
            }
            
            std::cout << "Client: Transaction complete" << std::endl;
        } else {
            std::cerr << "Client: Failed to connect" << std::endl;
        }
        
        close(sock);
    });
    
    // Timeout thread
    auto test_done = test_done_promise.get_future();
    std::thread timer_thread([loop, &test_done]() {
        if (test_done.wait_for(std::chrono::seconds(10)) == std::future_status::timeout) {
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
    
    std::cout << "Messages processed: " << messages_processed.load() << "/" << EXPECTED_MESSAGES << std::endl;
    
    if (test_passed.load() && messages_processed.load() >= EXPECTED_MESSAGES) {
        std::cout << "=== FULL_TRANSACTION_TEST_SUCCESS ===" << std::endl;
        return 0;
    } else {
        std::cout << "=== FULL_TRANSACTION_TEST_FAILED ===" << std::endl;
        return 1;
    }
}
