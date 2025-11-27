#ifndef SMTPSERVER_GENERATED_SMTP_HANDLER_HPP
#define SMTPSERVER_GENERATED_SMTP_HANDLER_HPP

#include "states.hpp"
#include "data_types.hpp"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace smtpserver_generated {

/**
 * Server configuration - same as the interpreted version.
 */
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

/**
 * SMTPHandler - A handler class that satisfies smtp::generated::ServerHandlerConcept.
 *
 * IMPORTANT: This handler is shared across ALL connections. It must NOT store
 * any per-connection state as member variables. All connection-specific data
 * flows through the message input/output parameters.
 *
 * ALL METHODS ARE CONST to enforce this constraint at compile-time.
 *
 * This handler implements on_<StateName>() overloaded methods for each message
 * type that can arrive at that state. The runner uses std::visit + ADL to
 * dispatch to the correct overload.
 */
class SMTPHandler {
public:
    explicit SMTPHandler(const ServerConfiguration& config);

    // Initial state handler - sends greeting (no input needed)
    smtp::generated::OpenOutput on_Open() const;

    // AwaitServerEHLOResponse state - receives EHLO command
    smtp::generated::AwaitServerEHLOResponseOutput 
    on_AwaitServerEHLOResponse(const smtp::generated::SMTPEHLOCommandData& msg) const;

    // AwaitServerMAILFROMResponse state - receives MAIL FROM command
    smtp::generated::AwaitServerMAILFROMResponseOutput 
    on_AwaitServerMAILFROMResponse(const smtp::generated::SMTPMAILFROMCommandData& msg) const;

    // AwaitServerRCPTTOResponse state - receives first RCPT TO or additional RCPT TO
    smtp::generated::AwaitServerRCPTTOResponseOutput 
    on_AwaitServerRCPTTOResponse(const smtp::generated::SMTPRCPTTOCommandData& msg) const;
    
    smtp::generated::AwaitServerRCPTTOResponseOutput 
    on_AwaitServerRCPTTOResponse(const smtp::generated::AdditionalSMTPRCPTTOCommandData& msg) const;

    // AwaitServerDATAResponse state - receives DATA command
    smtp::generated::AwaitServerDATAResponseOutput 
    on_AwaitServerDATAResponse(const smtp::generated::SMTPDATACommandData& msg) const;

    // AwaitServerDATAContentResponse state - receives DATA content
    smtp::generated::AwaitServerDATAContentResponseOutput 
    on_AwaitServerDATAContentResponse(const smtp::generated::SMTPDATAContentData& msg) const;

    // AwaitServerQUITResponse state - receives QUIT commands from various states
    smtp::generated::AwaitServerQUITResponseOutput 
    on_AwaitServerQUITResponse(const smtp::generated::SMTPQUITCommandData& msg) const;
    
    smtp::generated::AwaitServerQUITResponseOutput 
    on_AwaitServerQUITResponse(const smtp::generated::SMTPQUITCommandFromEHLOData& msg) const;
    
    smtp::generated::AwaitServerQUITResponseOutput 
    on_AwaitServerQUITResponse(const smtp::generated::SMTPQUITCommandFromFirstRCPTTOData& msg) const;
    
    smtp::generated::AwaitServerQUITResponseOutput 
    on_AwaitServerQUITResponse(const smtp::generated::SMTPQUITCommandFromRCPTTOOrDATAData& msg) const;

private:
    const ServerConfiguration& config_;
    
    // NOTE: No per-connection state here! All connection data flows through messages.
    
    // Helper to check if sender is blocked
    bool is_sender_blocked(const std::string& sender) const;
    
    // Helper to check if recipient is valid
    bool is_recipient_valid(const std::string& recipient) const;
};

} // namespace smtpserver_generated

#endif // SMTPSERVER_GENERATED_SMTP_HANDLER_HPP
