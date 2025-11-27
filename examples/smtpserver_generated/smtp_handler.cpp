#include "smtp_handler.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>

namespace smtpserver_generated {

namespace {

// Helper function to split an email address into user and domain parts.
std::pair<std::string, std::string> split_email(const std::string& email) {
    auto at_pos = email.find('@');
    if (at_pos == std::string::npos) {
        return {"", email};  // No @ found, treat whole thing as domain
    }
    return {email.substr(0, at_pos), email.substr(at_pos + 1)};
}

} // anonymous namespace

SMTPHandler::SMTPHandler(const ServerConfiguration& config)
    : config_(config) {}

smtp::generated::OpenOutput SMTPHandler::on_Open() const {
    std::cerr << "Connection opened by client" << std::endl;
    
    smtp::generated::SMTPServerGreetingData greeting;
    greeting.code_tens = 22;
    greeting.msg = "Welcome to " + config_.server_name;
    return greeting;
}

smtp::generated::AwaitServerEHLOResponseOutput 
SMTPHandler::on_AwaitServerEHLOResponse(const smtp::generated::SMTPEHLOCommandData& cmd) const {
    std::cerr << "Received EHLO from: " << cmd.client_domain << std::endl;
    
    // Check if the client domain is blocked
    if (config_.blocked_client_domains.find(cmd.client_domain) 
            != config_.blocked_client_domains.end()) {
        smtp::generated::SMTPEHLOFailureResponseData failure;
        failure.code_tens = 55;
        failure.msg = "Bad client, go away!";
        return failure;
    }
    
    smtp::generated::SMTPEHLOSuccessResponseData success;
    success.client_domain = cmd.client_domain;
    success.code_tens = 50;
    success.msg = "Hello, pleased to meet you";
    return success;
}

smtp::generated::AwaitServerMAILFROMResponseOutput 
SMTPHandler::on_AwaitServerMAILFROMResponse(const smtp::generated::SMTPMAILFROMCommandData& cmd) const {
    std::cerr << "Received MAIL FROM: " << cmd.sender << std::endl;
    
    // Check if the sender is blocked
    if (is_sender_blocked(cmd.sender)) {
        smtp::generated::SMTPMAILFROMFailureResponseData failure;
        failure.client_domain = cmd.client_domain;
        failure.code_tens = 55;
        failure.msg = "Blocked sender";
        return failure;
    }
    
    smtp::generated::SMTPMAILFROMSuccessResponseData success;
    success.client_domain = cmd.client_domain;
    success.sender = cmd.sender;
    success.code_tens = 50;
    success.msg = "Sender OK";
    return success;
}

smtp::generated::AwaitServerRCPTTOResponseOutput 
SMTPHandler::on_AwaitServerRCPTTOResponse(const smtp::generated::SMTPRCPTTOCommandData& cmd) const {
    std::cerr << "Received RCPT TO: " << cmd.recipient << std::endl;
    
    // Check if this recipient is valid
    if (!is_recipient_valid(cmd.recipient)) {
        smtp::generated::SMTPRCPTTOFailureResponseData failure;
        failure.client_domain = cmd.client_domain;
        failure.sender = cmd.sender;
        failure.recipient_list = cmd.recipient_list;
        failure.code_tens = 55;
        failure.msg = "Invalid recipient";
        return failure;
    }
    
    // Build success response - recipient_list from input already contains previous recipients
    smtp::generated::SMTPRCPTTOSuccessResponseData success;
    success.client_domain = cmd.client_domain;
    success.sender = cmd.sender;
    success.code_tens = 50;
    success.msg = "Recipient OK";
    
    // Add the new recipient to the list
    smtp::generated::SMTPRCPTTOSuccessResponse_recipient_listElement elem;
    elem.recipient = cmd.recipient;
    elem.parameters = "";
    success.recipient_list.push_back(elem);
    
    return success;
}

smtp::generated::AwaitServerQUITResponseOutput 
SMTPHandler::on_AwaitServerQUITResponse(
        const smtp::generated::SMTPQUITCommandFromFirstRCPTTOData& cmd) const {
    std::cerr << "Received QUIT command" << std::endl;
    smtp::generated::SMTPQUITResponseData response;
    response.code_tens = 21;
    response.msg = "Closing connection, goodbye";
    return response;
}

smtp::generated::AwaitServerRCPTTOResponseOutput 
SMTPHandler::on_AwaitServerRCPTTOResponse(
        const smtp::generated::AdditionalSMTPRCPTTOCommandData& cmd) const {
    std::cerr << "Received additional RCPT TO: " << cmd.recipient << std::endl;
    
    // Check if this recipient is valid
    if (!is_recipient_valid(cmd.recipient)) {
        smtp::generated::SMTPRCPTTOFailureResponseData failure;
        failure.client_domain = cmd.client_domain;
        failure.sender = cmd.sender;
        // Convert recipient list
        for (const auto& r : cmd.recipient_list) {
            failure.recipient_list.push_back(r.recipient);
        }
        failure.code_tens = 55;
        failure.msg = "Invalid recipient";
        return failure;
    }
    
    // Build success response - copy existing recipients and add the new one
    smtp::generated::SMTPRCPTTOSuccessResponseData success;
    success.client_domain = cmd.client_domain;
    success.sender = cmd.sender;
    success.code_tens = 50;
    success.msg = "Recipient OK";
    
    // Copy existing recipients from input
    for (const auto& r : cmd.recipient_list) {
        smtp::generated::SMTPRCPTTOSuccessResponse_recipient_listElement elem;
        elem.recipient = r.recipient;
        elem.parameters = r.parameters;
        success.recipient_list.push_back(elem);
    }
    
    // Add the new recipient
    smtp::generated::SMTPRCPTTOSuccessResponse_recipient_listElement elem;
    elem.recipient = cmd.recipient;
    elem.parameters = "";
    success.recipient_list.push_back(elem);
    
    return success;
}

smtp::generated::AwaitServerDATAResponseOutput 
SMTPHandler::on_AwaitServerDATAResponse(const smtp::generated::SMTPDATACommandData& cmd) const {
    std::cerr << "Received DATA command" << std::endl;
    
    smtp::generated::SMTPDATAResponseData response;
    response.client_domain = cmd.client_domain;
    response.sender = cmd.sender;
    response.recipient_list = cmd.recipient_list;
    response.code_tens = 54;
    response.msg = "Start mail input; end with <CRLF>.<CRLF>";
    return response;
}

smtp::generated::AwaitServerQUITResponseOutput 
SMTPHandler::on_AwaitServerQUITResponse(
        const smtp::generated::SMTPQUITCommandFromRCPTTOOrDATAData& cmd) const {
    std::cerr << "Received QUIT command" << std::endl;
    smtp::generated::SMTPQUITResponseData response;
    response.code_tens = 21;
    response.msg = "Closing connection, goodbye";
    return response;
}

smtp::generated::AwaitServerDATAContentResponseOutput 
SMTPHandler::on_AwaitServerDATAContentResponse(const smtp::generated::SMTPDATAContentData& data) const {
    std::cerr << "Received DATA content (" << data.content.size() << " bytes)" << std::endl;
    
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()).count();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    
    // Build email content with Received header
    std::ostringstream content_stream;
    content_stream << "Received: from " << data.sender
                  << " by " << data.client_domain
                  << " with SMTP; " << now_time_t << "\n";
    content_stream << data.content;
    std::string full_content = content_stream.str();
    
    // Write to each recipient's maildir
    for (const auto& recipient : data.recipient_list) {
        auto [recipient_user, recipient_domain] = split_email(recipient);
        
        // Build maildir path
        std::filesystem::path maildir_path = config_.maildir;
        maildir_path /= recipient_domain;
        maildir_path /= recipient_user;
        maildir_path /= "new";
        
        // Create directories if needed
        std::filesystem::create_directories(maildir_path);
        
        // Generate unique filename
        std::filesystem::path filename = maildir_path / 
            (std::to_string(now_ms) + ".mail");
        
        std::cerr << "Writing email to " << filename << std::endl;
        
        std::ofstream ofs(filename);
        if (ofs) {
            ofs << full_content;
        } else {
            std::cerr << "Error writing email to file: " << filename << std::endl;
        }
    }
    
    smtp::generated::SMTPDATAWrittenData written;
    written.client_domain = data.client_domain;
    written.code_tens = 50;
    written.msg = "Message accepted for delivery";
    return written;
}

smtp::generated::AwaitServerQUITResponseOutput 
SMTPHandler::on_AwaitServerQUITResponse(const smtp::generated::SMTPQUITCommandData& msg) const {
    std::cerr << "Received QUIT command" << std::endl;
    smtp::generated::SMTPQUITResponseData response;
    response.code_tens = 21;
    response.msg = "Closing connection, goodbye";
    return response;
}

smtp::generated::AwaitServerQUITResponseOutput 
SMTPHandler::on_AwaitServerQUITResponse(const smtp::generated::SMTPQUITCommandFromEHLOData& msg) const {
    std::cerr << "Received QUIT command" << std::endl;
    smtp::generated::SMTPQUITResponseData response;
    response.code_tens = 21;
    response.msg = "Closing connection, goodbye";
    return response;
}

bool SMTPHandler::is_sender_blocked(const std::string& sender) const {
    auto [sender_user, sender_domain] = split_email(sender);
    auto it = config_.blocked_senders.find(sender_domain);
    return it != config_.blocked_senders.end() &&
           it->second.find(sender_user) != it->second.end();
}

bool SMTPHandler::is_recipient_valid(const std::string& recipient) const {
    auto [recipient_user, recipient_domain] = split_email(recipient);
    auto it = config_.valid_recipients.find(recipient_domain);
    return it != config_.valid_recipients.end() &&
           it->second.find(recipient_user) != it->second.end();
}

} // namespace smtpserver_generated
