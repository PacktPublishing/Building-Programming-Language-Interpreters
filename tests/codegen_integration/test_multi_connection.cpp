/**
 * Test multiple concurrent connections to the generated server.
 *
 * This test verifies that the server can handle multiple simultaneous
 * connections correctly, each with their own independent state machines.
 *
 * Multiple clients connect and perform SMTP transactions concurrently,
 * and all should complete successfully to the Closed state.
 */
#include "protocol.hpp"
#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>
#include <atomic>
#include <future>
#include <variant>
#include <vector>
#include <mutex>

#include <uv.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace smtp::generated;

// Test state tracking
constexpr int NUM_CLIENTS = 3;
std::atomic<int> connections_completed{0};
std::atomic<int> connections_closed{0};
std::mutex output_mutex;

/**
 * Handler for multi-connection test.
 */
struct MultiConnHandler {
    MultiConnHandler() = default;
    
    OpenOutput on_Open() const {
        SMTPServerGreetingData greeting;
        greeting.code_tens = 22;
        greeting.msg = "Multi-Connection Test Server";
        return greeting;
    }
    
    AwaitServerEHLOResponseOutput on_AwaitServerEHLOResponse(const SMTPEHLOCommandData& msg) const {
        SMTPEHLOSuccessResponseData response;
        response.client_domain = msg.client_domain;
        response.code_tens = 50;
        response.msg = "Hello " + msg.client_domain;
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
        connections_completed.fetch_add(1);
        SMTPQUITResponseData response;
        response.code_tens = 21;
        response.msg = "Goodbye";
        return response;
    }
    
    AwaitServerQUITResponseOutput on_AwaitServerQUITResponse(const SMTPQUITCommandFromEHLOData& msg) const {
        connections_completed.fetch_add(1);
        SMTPQUITResponseData response;
        response.code_tens = 21;
        response.msg = "Goodbye";
        return response;
    }
    
    AwaitServerQUITResponseOutput on_AwaitServerQUITResponse(const SMTPQUITCommandFromFirstRCPTTOData& msg) const {
        connections_completed.fetch_add(1);
        SMTPQUITResponseData response;
        response.code_tens = 21;
        response.msg = "Goodbye";
        return response;
    }
    
    AwaitServerQUITResponseOutput on_AwaitServerQUITResponse(const SMTPQUITCommandFromRCPTTOOrDATAData& msg) const {
        connections_completed.fetch_add(1);
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
    const MultiConnHandler handler;
    std::vector<std::unique_ptr<ServerRunner<MultiConnHandler>>> runners;
    std::vector<uv_tcp_t*> handles;
    std::mutex runners_mutex;
    std::atomic<bool> server_closing{false};
};

void on_close(uv_handle_t* handle) {
    connections_closed.fetch_add(1);
    // Note: don't delete the handle here, it's managed by the test
    
    // Check if all connections have closed
    if (connections_closed.load() >= NUM_CLIENTS) {
        uv_stop(uv_handle_get_loop(handle));
    }
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
    auto* runner = static_cast<ServerRunner<MultiConnHandler>*>(stream->data);
    
    if (nread > 0) {
        runner->on_bytes_received(std::string_view(buf->base, nread));
        process_output(*runner, reinterpret_cast<uv_tcp_t*>(stream));
        
        if (runner->is_closed()) {
            uv_close(reinterpret_cast<uv_handle_t*>(stream), on_close);
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
        auto runner = std::make_unique<ServerRunner<MultiConnHandler>>(state->handler);
        runner->start();
        process_output(*runner, client);
        
        client->data = runner.get();
        
        {
            std::lock_guard<std::mutex> lock(state->runners_mutex);
            state->runners.push_back(std::move(runner));
            state->handles.push_back(client);
        }
        
        uv_read_start(reinterpret_cast<uv_stream_t*>(client), alloc_buffer, on_read);
    } else {
        uv_close(reinterpret_cast<uv_handle_t*>(client), on_close);
    }
}

// Client function that performs a simple EHLO/QUIT transaction
void run_client(int port, int client_id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50 + client_id * 20));
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    
    if (connect(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0) {
        {
            std::lock_guard<std::mutex> lock(output_mutex);
            std::cout << "Client " << client_id << ": Connected" << std::endl;
        }
        
        char buf[4096];
        ssize_t n;
        
        // Read greeting
        n = recv(sock, buf, sizeof(buf), 0);
        
        // Send EHLO
        std::string ehlo = "EHLO client" + std::to_string(client_id) + ".example.com\r\n";
        send(sock, ehlo.c_str(), ehlo.size(), 0);
        
        // Read EHLO response
        n = recv(sock, buf, sizeof(buf), 0);
        
        // Send QUIT
        const char* quit = "QUIT\r\n";
        send(sock, quit, strlen(quit), 0);
        
        // Read QUIT response
        n = recv(sock, buf, sizeof(buf), 0);
        
        {
            std::lock_guard<std::mutex> lock(output_mutex);
            std::cout << "Client " << client_id << ": Transaction complete" << std::endl;
        }
    } else {
        std::lock_guard<std::mutex> lock(output_mutex);
        std::cerr << "Client " << client_id << ": Failed to connect" << std::endl;
    }
    
    close(sock);
}

int main() {
    std::cout << "=== Testing Multiple Concurrent Connections ===" << std::endl;
    
    uv_loop_t* loop = uv_default_loop();
    
    ServerState state;
    state.loop = loop;
    
    uv_tcp_init(loop, &state.server);
    state.server.data = &state;
    
    struct sockaddr_in addr;
    uv_ip4_addr("127.0.0.1", 0, &addr);
    
    uv_tcp_bind(&state.server, reinterpret_cast<const struct sockaddr*>(&addr), 0);
    uv_listen(reinterpret_cast<uv_stream_t*>(&state.server), NUM_CLIENTS, on_connection);
    
    // Get the actual port
    struct sockaddr_storage storage;
    int namelen = sizeof(storage);
    uv_tcp_getsockname(&state.server, reinterpret_cast<struct sockaddr*>(&storage), &namelen);
    int port = ntohs(reinterpret_cast<struct sockaddr_in*>(&storage)->sin_port);
    
    std::cout << "Server listening on port " << port << " for " << NUM_CLIENTS << " clients" << std::endl;
    
    // Start client threads
    std::vector<std::thread> client_threads;
    for (int i = 0; i < NUM_CLIENTS; ++i) {
        client_threads.emplace_back(run_client, port, i);
    }
    
    // Timeout thread
    std::thread timer_thread([loop]() {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cout << "Test timed out!" << std::endl;
        uv_stop(loop);
    });
    timer_thread.detach();
    
    uv_run(loop, UV_RUN_DEFAULT);
    
    for (auto& t : client_threads) {
        t.join();
    }
    
    // Cleanup server handle
    uv_close(reinterpret_cast<uv_handle_t*>(&state.server), nullptr);
    uv_run(loop, UV_RUN_NOWAIT);
    
    // Clean up client handles (they're closed but memory needs to be freed)
    {
        std::lock_guard<std::mutex> lock(state.runners_mutex);
        for (auto* handle : state.handles) {
            delete handle;
        }
        state.handles.clear();
    }
    
    std::cout << "Connections completed: " << connections_completed.load() << "/" << NUM_CLIENTS << std::endl;
    std::cout << "Connections closed: " << connections_closed.load() << "/" << NUM_CLIENTS << std::endl;
    
    if (connections_completed.load() == NUM_CLIENTS && connections_closed.load() == NUM_CLIENTS) {
        std::cout << "=== MULTI_CONNECTION_TEST_SUCCESS ===" << std::endl;
        return 0;
    } else {
        std::cout << "=== MULTI_CONNECTION_TEST_FAILED ===" << std::endl;
        return 1;
    }
}
