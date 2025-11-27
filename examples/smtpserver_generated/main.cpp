/**
 * SMTP Server using Generated Protocol Code
 *
 * This example demonstrates using the code generator to create a type-safe
 * SMTP server with concept-based callbacks instead of the interpreted approach.
 *
 * The generated code provides:
 * - Type-safe data structures for each message type
 * - ServerRunner template that dispatches to handler methods
 * - Compile-time verification that all required handlers are implemented
 *
 * This example uses GeneratedServerWrapper from networkprotocoldsl_uv
 * to handle all the libuv integration automatically.
 */

#include "smtp_handler.hpp"
#include "runner.hpp"

#include <networkprotocoldsl_uv/asyncworkqueue.hpp>
#include <networkprotocoldsl_uv/generatedserverwrapper.hpp>

#include <CLI/CLI.hpp>

#include <uv.h>

#include <condition_variable>
#include <csignal>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace {

// Signal handling
std::mutex signal_mtx;
std::condition_variable signal_cv;
bool stop_signal_received = false;

void signal_handler(int signum) {
    std::lock_guard<std::mutex> lock(signal_mtx);
    stop_signal_received = true;
    std::cerr << "Received signal: " << signum << std::endl;
    signal_cv.notify_one();
}

} // anonymous namespace

int main(int argc, char* argv[]) {
    std::cerr << "SMTP Server (Generated) example started." << std::endl;

    // Parse command line arguments
    smtpserver_generated::ServerConfiguration config;
    
    // Add some sample values to the configuration
    config.valid_recipients["example.com"].insert("user1");
    config.valid_recipients["example.com"].insert("user2");
    config.valid_recipients["other.com"].insert("user3");
    config.blocked_senders["bad.com"].insert("user");
    config.blocked_client_domains.insert("bad.com");
    
    CLI::App app{"SMTP Server (Generated)"};
    app.add_option("--bind-ip", config.bind_ip, "IP address to bind")
        ->default_val("127.0.0.1");
    app.add_option("--bind-port", config.bind_port, "Port to bind")
        ->default_val(8025);
    app.add_option("--server-name", config.server_name, "Server name")
        ->default_val("testsrv");
    app.add_option("--maildir", config.maildir, "Mail directory")
        ->default_val("./maildir/");
    
    CLI11_PARSE(app, argc, argv);
    
    std::cerr << "Binding to " << config.bind_ip << ":" << config.bind_port << std::endl;
    
    // Set up libuv
    uv_loop_t* loop = uv_default_loop();
    
    // Create async work queue
    networkprotocoldsl_uv::AsyncWorkQueue async_queue(loop);
    
    // Create the shared handler - holds configuration, shared across all connections
    smtpserver_generated::SMTPHandler handler(config);
    
    // Create server wrapper using the generated runner
    // The handler is shared across all connections; each connection gets its own Runner
    using Runner = smtp::generated::ServerRunner<smtpserver_generated::SMTPHandler>;
    using Handler = smtpserver_generated::SMTPHandler;
    networkprotocoldsl_uv::GeneratedServerWrapper<Runner, Handler> server(
        handler, async_queue);
    
    // Start the server
    auto bind_future = server.start(config.bind_ip, config.bind_port);
    
    // Setup signal handling
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    // Run the loop in a separate thread
    std::thread io_thread([loop]() {
        uv_run(loop, UV_RUN_DEFAULT);
    });
    
    // Check bind result
    auto bind_result = bind_future.get();
    if (std::holds_alternative<std::string>(bind_result)) {
        std::cerr << "Failed to bind: " << std::get<std::string>(bind_result) << std::endl;
        uv_stop(loop);
        io_thread.join();
        return 1;
    }
    
    std::cerr << "Server listening on " << config.bind_ip << ":" << config.bind_port 
              << " (fd=" << std::get<int>(bind_result) << ")" << std::endl;
    
    // Wait for signal
    std::cerr << "Server is running; press Ctrl+C to stop." << std::endl;
    {
        std::unique_lock<std::mutex> lock(signal_mtx);
        signal_cv.wait(lock, [] { return stop_signal_received; });
    }
    
    // Stop the server
    std::cerr << "Stopping server..." << std::endl;
    server.stop();
    
    uv_stop(loop);
    io_thread.join();
    
    std::cerr << "Server stopped." << std::endl;
    return 0;
}
