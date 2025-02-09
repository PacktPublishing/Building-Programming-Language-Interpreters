#ifndef NETWORKPROTOCOLDSL_INTERPRETEDPROGRAM_HPP
#define NETWORKPROTOCOLDSL_INTERPRETEDPROGRAM_HPP

#include <networkprotocoldsl/interpreter.hpp>
#include <networkprotocoldsl/lexicalpad.hpp>
#include <networkprotocoldsl/operation.hpp>
#include <networkprotocoldsl/optree.hpp>

#include <memory>
#include <string>

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
  static std::optional<InterpretedProgram>
  generate_client(const std::string &sf);
  static std::optional<InterpretedProgram>
  generate_server(const std::string &sf);

  /**
   * Skips the parsing and receives an optree instead.
   */
  InterpretedProgram(std::shared_ptr<const OpTree> o)
      : source_file("-"), optree(o) {}

  /**
   * Obtains an instance of an interpreter for this program.
   */
  Interpreter get_instance(std::optional<Value> arglist = std::nullopt) {
    std::shared_ptr<LexicalPad> rootpad =
        std::make_shared<LexicalPad>(LexicalPad());
    if (arglist.has_value()) {
      rootpad->initialize_global("argv", arglist.value());
    }
    return Interpreter(optree, rootpad);
  };
};

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_INTERPRETER_HPP
