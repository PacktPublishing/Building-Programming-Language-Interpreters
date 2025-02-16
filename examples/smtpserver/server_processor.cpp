#include "server_processor.hpp"
#include <iostream>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

// Function to split an email address into user and domain parts.
std::pair<std::string, std::string> split_email(const std::string &email) {
  auto at_pos = email.find('@');
  if (at_pos == std::string::npos) {
    throw std::invalid_argument("Invalid email address");
  }
  return {email.substr(0, at_pos), email.substr(at_pos + 1)};
}

// Include the necessary DSL headers.
#include <networkprotocoldsl/interpreterrunner.hpp>
#include <networkprotocoldsl/value.hpp>

namespace smtpserver {
using namespace networkprotocoldsl;

// Helper function to wrap a string in an Octets value.
static value::Octets _o(const std::string &str) {
  return value::Octets{std::make_shared<std::string>(str)};
}

static Value onOpen(const ServerConfiguration &config,
                    const std::vector<Value> &args) {
  std::cerr << "Connection opened by client" << std::endl;
  return value::DynamicList{
      _o("SMTP Server Greeting"),
      value::Dictionary{
          {{"code_tens", 22}, {"msg", _o("Welcome to the SMTP server")}}}};
}

// Called when the server is in state "AwaitServerEHLOResponse".
// Sends the EHLO response.
static Value onAwaitServerEHLOResponse(const ServerConfiguration &config,
                                       const std::vector<Value> &args) {
  // Extract the data dictionary from the incoming message.
  value::Dictionary dict = std::get<value::Dictionary>(args[0]);
  auto client_domain_member = dict.members->at("client_domain");
  std::string client_domain =
      *std::get<value::Octets>(client_domain_member).data;
  if (config.blocked_client_domains.find(client_domain) !=
      config.blocked_client_domains.end()) {
    return value::DynamicList{
        {_o("SMTP EHLO Failure Response"),
         value::Dictionary{{{"client_domain", client_domain_member},
                            {"code_tens", 55},
                            {"msg", _o("Bad client, go away!")}}}}};
  }

  return value::DynamicList{
      {_o("SMTP EHLO Success Response"),
       value::Dictionary{{{"client_domain", client_domain_member},
                          {"code_tens", 50},
                          {"msg", _o("Hello, pleased to meet you")}}}}};
}

// Called when the server is in state "AwaitServerMAILFROMResponse".
// Processes the MAIL FROM command by accepting the sender.
static Value onAwaitServerMAILFROMResponse(const ServerConfiguration &config,
                                           const std::vector<Value> &args) {
  // Extract the data dictionary from the incoming message.
  value::Dictionary dict = std::get<value::Dictionary>(args[0]);
  auto client_domain = dict.members->at("client_domain");
  std::string sender =
      *std::get<value::Octets>(dict.members->at("sender")).data;

  // check if the user is blocked
  auto [sender_user, sender_domain] = split_email(sender);
  auto it = config.blocked_senders.find(sender_domain);
  if (it != config.blocked_senders.end() &&
      it->second.find(sender_user) != it->second.end()) {
    return value::DynamicList{
        {_o("SMTP MAIL FROM Failure Response"),
         value::Dictionary{{{"client_domain", client_domain},
                            {"sender", _o(sender)},
                            {"code_tens", 55},
                            {"msg", _o("Blocked sender")}}}}};
  }

  return value::DynamicList{
      {_o("SMTP MAIL FROM Success Response"),
       value::Dictionary{{{"client_domain", client_domain},
                          {"sender", _o(sender)},
                          {"code_tens", 50},
                          {"msg", _o("Sender OK")}}}}};
}

// Called when the server is in state "AwaitServerRCPTTOResponse".
static Value onAwaitServerRCPTTOResponse(const ServerConfiguration &config,
                                         const std::vector<Value> &args) {
  // Extract the data dictionary from the incoming message.
  value::Dictionary dict = std::get<value::Dictionary>(args[0]);
  auto client_domain = dict.members->at("client_domain");
  auto sender = dict.members->at("sender");
  std::string recipient =
      *std::get<value::Octets>(dict.members->at("recipient")).data;

  // Check if this recipient is in the list of valid recipients.
  auto [recipient_user, recipient_domain] = split_email(recipient);
  auto it = config.valid_recipients.find(recipient_domain);
  if (it == config.valid_recipients.end() ||
      it->second.find(recipient_user) == it->second.end()) {
    return value::DynamicList{
        {_o("SMTP RCPT TO Failure Response"),
         value::Dictionary{{{"client_domain", client_domain},
                            {"sender", sender},
                            {"code_tens", 55},
                            {"msg", _o("Invalid recipient")}}}}};
  }

  Value recipient_list;
  auto r_it = dict.members->find("recipient_list");
  if (r_it != dict.members->end()) {
    recipient_list = r_it->second;
    if (!std::holds_alternative<value::DynamicList>(recipient_list)) {
      recipient_list = value::DynamicList{{_o(recipient)}};
    } else {
      auto acc = std::make_shared<std::vector<Value>>(
          *(std::get<value::DynamicList>(recipient_list).values));
      acc->push_back(_o(recipient));
      recipient_list = value::DynamicList{acc};
    }
  } else {
    recipient_list = value::DynamicList{{_o(recipient)}};
  }

  return value::DynamicList{
      {_o("SMTP RCPT TO Success Response"),
       value::Dictionary{{{"client_domain", client_domain},
                          {"sender", sender},
                          {"recipient_list", recipient_list},
                          {"code_tens", 50},
                          {"msg", _o("Recipient OK")}}}}};
}

// Called when the server is in state "AwaitServerDATAResponse".
// Sends the response to the DATA command.
static Value onAwaitServerDATAResponse(const ServerConfiguration &config,
                                       const std::vector<Value> &args) {
  // Extract the data dictionary from the incoming message.
  value::Dictionary dict = std::get<value::Dictionary>(args[0]);
  auto client_domain = dict.members->at("client_domain");
  auto sender = dict.members->at("sender");
  auto recipient_list = dict.members->at("recipient_list");

  return value::DynamicList{
      {_o("SMTP DATA Response"),
       value::Dictionary{
           {{"client_domain", client_domain},
            {"sender", sender},
            {"recipient_list", recipient_list},
            {"code_tens", 54},
            {"msg", _o("Start mail input; end with <CRLF>.<CRLF>")}}}}};
}

// Called when the server is in state "AwaitServerDATAContent".
// Processes the DATA content sent by the client and writes it into a maildir.
static Value onAwaitServerDATAContentResponse(const ServerConfiguration &config,
                                              const std::vector<Value> &args) {
  // Extract the data dictionary from the incoming message.
  value::Dictionary dict = std::get<value::Dictionary>(args[0]);
  auto client_domain = dict.members->at("client_domain");
  auto sender = dict.members->at("sender");
  auto recipient_list =
      std::get<value::DynamicList>(dict.members->at("recipient_list"));

  auto now = std::chrono::system_clock::now();
  auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch())
                    .count();
  // generate the time in the format for the received header
  auto now_time_t = std::chrono::system_clock::to_time_t(now);

  // iterate over the recipient_list and drop a new message on the user's new
  // maildir. This server is will use ${config.maildir}/domain/user/new/ as the
  // directory where the new mail will be stored.
  auto content = *std::get<value::Octets>(dict.members->at("content")).data;
  auto content_sstream = std::stringstream();
  // insert the "Received" header before the content
  content_sstream << "Received: from " << *std::get<value::Octets>(sender).data
                  << " by " << *std::get<value::Octets>(client_domain).data
                  << " with SMTP; " << now_time_t << "\n";
  content_sstream << content;
  for (const auto &recipient : *(recipient_list.values)) {
    auto recipient_email = *std::get<value::Octets>(recipient).data;
    auto [recipient_user, recipient_domain] = split_email(recipient_email);
    auto path_parts =
        std::vector<std::string>{recipient_domain, recipient_user, "new"};
    std::string maildir =
        std::accumulate(path_parts.begin(), path_parts.end(), config.maildir,
                        [](const std::string &acc, const std::string &part) {
                          auto target = acc + "/" + part;
                          // check if acc exists, if not create it as a
                          // directory then check if target exists, if not
                          // create it as a directory
                          if (!std::filesystem::exists(acc)) {
                            std::filesystem::create_directory(acc);
                          }
                          if (!std::filesystem::exists(target)) {
                            std::filesystem::create_directory(target);
                          }
                          return target;
                        }) +
        "/";
    // make sure the directory exists
    // Generate a unique filename based on the current time.
    std::string filename = maildir + std::to_string(now_ms) + ".mail";
    std::cerr << "Writing email to " << filename << std::endl;

    std::ofstream ofs(filename);
    if (!ofs) {
      std::cerr << "Error writing email to file: " << filename << std::endl;
      return value::DynamicList{
          {_o("SMTP DATA Write Error"),
           value::Dictionary{{"client_domain", client_domain}}}};
    }
    ofs << content_sstream.str();
    ofs.close();
  }

  return value::DynamicList{
      {_o("SMTP DATA Written"),
       value::Dictionary{{{"client_domain", client_domain},
                          {"code_tens", 50},
                          {"msg", _o("Message accepted for delivery")}}}}};
}

// Called when the server is in state "AwaitServerQUITResponse".
// Sends the QUIT response.
static Value onAwaitServerQUITResponse(const ServerConfiguration &config,
                                       const std::vector<Value> &args) {
  return value::DynamicList{
      {_o("SMTP QUIT Response"),
       value::Dictionary{
           {{"code_tens", 21}, {"msg", _o("Closing connection, goodbye")}}}}};
}

// Called when the server enters the "Closed" state.
static Value onClosed(const ServerConfiguration &config,
                      const std::vector<Value> &args) {
  std::cerr << "Connection closed by client" << std::endl;
  return value::DynamicList{{_o("Closed"), value::Dictionary{}}};
}

networkprotocoldsl::InterpreterRunner::callback_map
get_sever_callbacks(const ServerConfiguration &config) {
  // Build the server callback map using the on$state naming convention.
  return InterpreterRunner::callback_map{
      {"Open",
       [&config](const std::vector<Value> &args) -> Value {
         return onOpen(config, args);
       }},
      {"AwaitServerEHLOResponse",
       [&config](const std::vector<Value> &args) -> Value {
         return onAwaitServerEHLOResponse(config, args);
       }},
      {"AwaitServerMAILFROMResponse",
       [&config](const std::vector<Value> &args) -> Value {
         return onAwaitServerMAILFROMResponse(config, args);
       }},
      {"AwaitServerRCPTTOResponse",
       [&config](const std::vector<Value> &args) -> Value {
         return onAwaitServerRCPTTOResponse(config, args);
       }},
      {"AwaitServerDATAResponse",
       [&config](const std::vector<Value> &args) -> Value {
         return onAwaitServerDATAResponse(config, args);
       }},
      {"AwaitServerDATAContentResponse",
       [&config](const std::vector<Value> &args) -> Value {
         return onAwaitServerDATAContentResponse(config, args);
       }},
      {"AwaitServerQUITResponse",
       [&config](const std::vector<Value> &args) -> Value {
         return onAwaitServerQUITResponse(config, args);
       }},
      {"Closed", [&config](const std::vector<Value> &args) -> Value {
         return onClosed(config, args);
       }}};
}

} // namespace smtpserver
