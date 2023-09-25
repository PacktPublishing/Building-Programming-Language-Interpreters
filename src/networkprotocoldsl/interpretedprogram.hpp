#ifndef NETWORKPROTOCOLDSL_INTERPRETEDPROGRAM_HPP
#define NETWORKPROTOCOLDSL_INTERPRETERPROGRAM_HPP

#include <string>

#include <networkprotocoldsl/interpreter.hpp>

namespace networkprotocoldsl {

/***
 * The InterpretedProgram object represents the usage of a single program.
 *
 * Many instances of an interpreter can be spawned for a
 * program. This is where the parsing of the programming language
 * will take place, and from which a new instance can be
 * instantiated to deal with a specific connection.
 */
class InterpretedProgram {
  std::string source_file;

public:
  /***
   * Parses source code and creates the object that can be used to
   * instantiate interpreters for this specific code.
   */
  InterpretedProgram(std::string sf) : source_file(sf) {}

  /***
   * Obtains an instance of an interpreter for this program.
   */
  Interpreter get_instance() { return Interpreter(); };
};

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_INTERPRETER_HPP
