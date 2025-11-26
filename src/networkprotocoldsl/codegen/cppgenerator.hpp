#ifndef INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_CPPGENERATOR_HPP
#define INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_CPPGENERATOR_HPP

#include <networkprotocoldsl/codegen/outputcontext.hpp>
#include <networkprotocoldsl/codegen/protocolinfo.hpp>
#include <networkprotocoldsl/sema/ast/protocol.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace networkprotocoldsl::codegen {

/**
 * CppGenerator generates C++ code from a Protocol AST.
 *
 * The generator produces a library with .hpp/.cpp pairs for each unit,
 * using fixed entry point names within the specified namespace. This allows
 * integration code to work with any protocol using the same conventions.
 *
 * Generated files:
 *   - protocol.hpp           - Main include, aggregates all headers
 *   - data_types.hpp/.cpp    - Data structs for each message
 *   - states.hpp/.cpp        - State enum and transition types
 *   - parser.hpp/.cpp        - Sans-IO protocol parser
 *   - serializer.hpp/.cpp    - Sans-IO protocol serializer
 *   - state_machine.hpp/.cpp - State machine coordinator
 *   - CMakeLists.txt         - CMake build configuration (optional)
 *
 * Fixed entry points (within target namespace):
 *   - State              - Enum of all protocol states
 *   - Parser             - Sans-IO parser class
 *   - Serializer         - Sans-IO serializer class
 *   - ClientStateMachine - Client-side state machine template
 *   - ServerStateMachine - Server-side state machine template
 */
class CppGenerator {
public:
  /**
   * Construct a CppGenerator.
   *
   * @param protocol The analyzed protocol AST to generate code from
   * @param target_namespace The C++ namespace for generated code
   *                         (e.g., "myapp::smtp" or "protocols::http")
   * @param target_directory The output directory for generated files
   * @param library_name Optional name for the generated static library.
   *                     If provided, a CMakeLists.txt will be generated.
   */
  CppGenerator(std::shared_ptr<const sema::ast::Protocol> protocol,
               std::string target_namespace,
               std::filesystem::path target_directory,
               std::string library_name = "");

  /**
   * Generate all output files.
   *
   * Creates the target directory if it doesn't exist and writes all
   * generated header files.
   *
   * @return true on success, false on failure
   */
  bool generate();

  /**
   * Get any error messages from the last generate() call.
   */
  const std::vector<std::string> &errors() const { return errors_; }

  /**
   * Get the output context (for testing/debugging).
   */
  const OutputContext &context() const { return ctx_; }

  /**
   * Get the protocol info (for testing/debugging).
   */
  const ProtocolInfo &info() const { return info_; }

private:
  OutputContext ctx_;
  ProtocolInfo info_;
  std::filesystem::path target_directory_;
  std::string library_name_;
  std::vector<std::string> errors_;

  // Write a single file to the target directory
  bool write_file(const std::string &filename, const std::string &content);

  // Write a header/source pair to the target directory
  bool write_pair(const std::string &basename,
                  const std::string &header,
                  const std::string &source);

  // Generate the main protocol.hpp header
  std::string generate_main_header();

  // Generate CMakeLists.txt for building as a static library
  std::string generate_cmake_lists();
};

} // namespace networkprotocoldsl::codegen

#endif // INCLUDED_NETWORKPROTOCOLDSL_CODEGEN_CPPGENERATOR_HPP
