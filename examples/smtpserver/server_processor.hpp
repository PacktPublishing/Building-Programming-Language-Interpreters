#ifndef SERVER_PROCESSOR_HPP
#define SERVER_PROCESSOR_HPP

#include "server_core.hpp"
#include <networkprotocoldsl/interpreterrunner.hpp>
// ...existing includes if needed...

namespace smtpserver {

networkprotocoldsl::InterpreterRunner::callback_map
get_sever_callbacks(const ServerConfiguration &config);

} // namespace smtpserver

#endif // SERVER_PROCESSOR_HPP
