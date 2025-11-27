#include <networkprotocoldsl/codegen/outputcontext.hpp>

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace networkprotocoldsl::codegen {

OutputContext::OutputContext(std::string target_namespace)
    : target_namespace_(std::move(target_namespace)) {}

std::string OutputContext::open_namespace() const {
  std::ostringstream oss;
  oss << "namespace " << target_namespace_ << " {\n";
  return oss.str();
}

std::string OutputContext::close_namespace() const {
  std::ostringstream oss;
  oss << "} // namespace " << target_namespace_ << "\n";
  return oss.str();
}

std::string OutputContext::header_guard(const std::string &filename) const {
  // Convert namespace and filename to a valid header guard
  // e.g., "myapp::smtp" + "data_types.hpp" -> "MYAPP_SMTP_DATA_TYPES_HPP"
  std::string guard = target_namespace_ + "_" + filename;
  std::transform(guard.begin(), guard.end(), guard.begin(), [](char c) {
    if (c == ':' || c == '/' || c == '\\' || c == '.') {
      return '_';
    }
    return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  });
  
  // Remove consecutive underscores
  std::string result;
  result.reserve(guard.size());
  bool last_was_underscore = false;
  for (char c : guard) {
    if (c == '_') {
      if (!last_was_underscore) {
        result += c;
        last_was_underscore = true;
      }
    } else {
      result += c;
      last_was_underscore = false;
    }
  }
  return "GENERATED_" + result;
}

std::string OutputContext::escape_string_literal(const std::string &s) {
  std::ostringstream oss;
  oss << '"';
  for (char c : s) {
    switch (c) {
    case '\r':
      oss << "\\r";
      break;
    case '\n':
      oss << "\\n";
      break;
    case '\t':
      oss << "\\t";
      break;
    case '\\':
      oss << "\\\\";
      break;
    case '"':
      oss << "\\\"";
      break;
    default:
      if (std::isprint(static_cast<unsigned char>(c))) {
        oss << c;
      } else {
        // Output as hex escape
        oss << "\\x" << std::hex << std::setfill('0') << std::setw(2)
            << static_cast<unsigned int>(static_cast<unsigned char>(c));
      }
      break;
    }
  }
  oss << '"';
  return oss.str();
}

} // namespace networkprotocoldsl::codegen
