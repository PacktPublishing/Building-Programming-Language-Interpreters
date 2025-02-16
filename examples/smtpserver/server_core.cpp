#include "server_core.hpp"
#include "server_processor.hpp"

#include <networkprotocoldsl_uv/libuvserverwrapper.hpp>

#include <CLI/CLI.hpp>

#include <condition_variable>
#include <csignal>
#include <future>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

namespace {

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

namespace smtpserver {

void parse_email_list(
    const std::string &str,
    std::unordered_map<std::string, std::unordered_set<std::string>> &map) {
  auto pos = str.find('@');
  if (pos == std::string::npos) {
    throw CLI::ValidationError("Invalid format for email");
  }
  map[str.substr(pos + 1)].insert(str.substr(0, pos));
}

std::optional<int>
main_server(const std::span<const char *> &args,
            const networkprotocoldsl::InterpretedProgram &program,
            networkprotocoldsl_uv::AsyncWorkQueue &async_queue) {
  // Setup CLI11 parser.
  ServerConfiguration config;

  // add some sample values to the configuration
  config.valid_recipients["example.com"].insert("user1");
  config.valid_recipients["example.com"].insert("user2");
  config.valid_recipients["other.com"].insert("user3");
  config.blocked_senders["bad.com"].insert("user");
  config.blocked_client_domains.insert("bad.com");

  CLI::App app{"Server configuration"};
  app.add_option("--bind-ip", config.bind_ip, "IP address to bind")
      ->default_val("127.0.0.1");
  app.add_option("--bind-port", config.bind_port, "Port to bind")
      ->default_val(8080);
  app.add_option("--server-name", config.server_name, "Server name")
      ->default_val("testsrv");
  app.add_option("--maildir", config.maildir, "Mail directory")
      ->default_val("./maildir/");
  app.add_option("--valid-recipients", config.valid_recipients,
                 "Valid recipients");
  app.add_option("--blocked-senders", config.blocked_senders,
                 "Blocked senders");
  app.add_option("--blocked-client-domains", config.blocked_client_domains,
                 "Blocked client domains");

  // Convert span to vector (skip program name).
  std::vector<std::string> args_cli(++args.begin(), args.end());
  try {
    app.parse(args_cli);
  } catch (const CLI::ParseError &e) {
    return app.exit(e);
  }

  std::cerr << "Binding to IP: " << config.bind_ip
            << ", Port: " << config.bind_port << std::endl;

  // Get the server callbacks.
  auto server_callbacks = get_sever_callbacks(config);

  // Create server wrapper and start the server.
  networkprotocoldsl_uv::LibuvServerWrapper server_wrapper(
      program, server_callbacks, async_queue);
  auto bind_future = server_wrapper.start(config.bind_ip, config.bind_port);
  bind_future.wait();
  auto bind_result = bind_future.get();
  if (std::holds_alternative<std::string>(bind_result)) {
    std::cerr << "Bind failed: " << std::get<std::string>(bind_result)
              << std::endl;
    return std::nullopt;
  }
  std::cerr << "Server started on descriptor: " << std::get<int>(bind_result)
            << std::endl;

  // Setup signal handling for SIGINT.
  std::signal(SIGINT, signal_handler);
  std::cerr << "Server is running; press Ctrl+C to stop." << std::endl;

  {
    std::unique_lock<std::mutex> lock(signal_mtx);
    signal_cv.wait(lock, [] { return stop_signal_received; });
  }

  std::cerr << "Stopping server... " << std::flush;
  server_wrapper.stop();

  std::cerr << "done" << std::endl;

  return 0;
}

} // namespace smtpserver
