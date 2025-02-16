#ifndef SMTPSERVER_INTERPRETED_PROGRAM_HPP
#define SMTPSERVER_INTERPRETED_PROGRAM_HPP

#include <networkprotocoldsl/interpretedprogram.hpp>
#include <optional>

namespace smtpserver {
// Loads the embedded static source and returns an interpreted program.
std::optional<networkprotocoldsl::InterpretedProgram>
load_interpreted_program();
} // namespace smtpserver

#endif // SMTPSERVER_INTERPRETED_PROGRAM_HPP
