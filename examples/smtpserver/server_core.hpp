#ifndef SMTPSERVER_SERVER_CORE_HPP
#define SMTPSERVER_SERVER_CORE_HPP

#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl_uv/asyncworkqueue.hpp>

#include <optional>
#include <span>

#include <unordered_map>
#include <unordered_set>

// ...existing code...
namespace smtpserver {

struct ServerConfiguration {
  std::string server_name;
  std::unordered_map<std::string, std::unordered_set<std::string>>
      valid_recipients;
  std::unordered_map<std::string, std::unordered_set<std::string>>
      blocked_senders;
  std::unordered_set<std::string> blocked_client_domains;
  std::string maildir;
  std::string bind_ip;
  int bind_port;
};

std::optional<int>
main_server(const std::span<const char *> &args,
            const networkprotocoldsl::InterpretedProgram &program,
            networkprotocoldsl_uv::AsyncWorkQueue &async_queue);
} // namespace smtpserver

#endif // SMTPSERVER_SERVER_CORE_HPP
