#ifndef INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_OUTPUTCONTEXT_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_OUTPUTCONTEXT_HPP

#include <sstream>
#include <string>

namespace networkprotocoldsl::codegen {

/**
 * OutputContext provides utilities for generating C++ code output.
 *
 * This class encapsulates namespace management, header guard generation,
 * and other common output formatting needs for the code generator.
 */
class OutputContext {
public:
  /**
   * Construct an OutputContext.
   *
   * @param target_namespace The C++ namespace for generated code
   *                         (e.g., "myapp::smtp" or "protocols::http")
   */
  explicit OutputContext(std::string target_namespace);

  /**
   * Get the opening namespace declaration.
   * @return String like "namespace myapp::smtp {\n"
   */
  std::string open_namespace() const;

  /**
   * Get the closing namespace declaration.
   * @return String like "} // namespace myapp::smtp\n"
   */
  std::string close_namespace() const;

  /**
   * Generate a header guard for a given filename.
   *
   * Converts namespace and filename to a valid header guard.
   * e.g., "myapp::smtp" + "data_types.hpp" -> "GENERATED_MYAPP_SMTP_DATA_TYPES_HPP"
   *
   * @param filename The filename (e.g., "data_types.hpp")
   * @return The header guard string
   */
  std::string header_guard(const std::string &filename) const;

  /**
   * Get the target namespace.
   */
  const std::string &target_namespace() const { return target_namespace_; }

  /**
   * Escape a string for use as a C++ string literal.
   *
   * Handles special characters like \r, \n, \t, etc.
   *
   * @param s The string to escape
   * @return A quoted C++ string literal (e.g., "\"hello\\r\\n\"")
   */
  static std::string escape_string_literal(const std::string &s);

private:
  std::string target_namespace_;
};

} // namespace networkprotocoldsl::codegen

#endif // INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_OUTPUTCONTEXT_HPP
