#include "interpreted_program.hpp"
#include <string>

namespace smtpserver {
std::optional<networkprotocoldsl::InterpretedProgram>
load_interpreted_program() {
  // Load the embedded static source code via the compile definition.
  std::string source_code = std::string(
#ifndef SMTP_NETWORKPROTOCOLDSL_LITERAL
#error "SMTP_NETWORKPROTOCOLDSL_LITERAL not defined"
#endif
#include SMTP_NETWORKPROTOCOLDSL_LITERAL
  );
  return networkprotocoldsl::InterpretedProgram::generate_server_from_source(
      source_code);
}
} // namespace smtpserver