#ifndef NETWORKPROTOCOLDSL_INTERPRETEDPROGRAM_HPP
#define NETWORKPROTOCOLDSL_INTERPRETERPROGRAM_HPP

#include <memory>
#include <string>

#include <networkprotocoldsl/interpreter.hpp>
#include <networkprotocoldsl/lexicalpad.hpp>
#include <networkprotocoldsl/operation.hpp>
#include <networkprotocoldsl/optree.hpp>

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
  std::shared_ptr<const OpTree> optree;

public:
  /**
   * Parses source code and creates the object that can be used to
   * instantiate interpreters for this specific code.
   */
  InterpretedProgram(std::string sf)
      : source_file(sf), optree(std::make_shared<const OpTree>(
                             OpTree({operation::Int32Literal(0), {}}))) {}

  /**
   * Skips the parsing and receives an optree instead.
   */
  InterpretedProgram(std::shared_ptr<const OpTree> o)
      : source_file("-"), optree(o) {}

  /**
   * Obtains an instance of an interpreter for this program.
   */
  Interpreter get_instance() {
    return Interpreter(optree, std::make_shared<LexicalPad>(LexicalPad()));
  };
};

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_INTERPRETER_HPP
