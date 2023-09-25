#ifndef NETWORKPROTOCOLDSL_INTERPRETER_HPP
#define NETWORKPROTOCOLDSL_INTERPRETER_HPP

namespace networkprotocoldsl {

/***
 * The InterpretedProgram object represents the usage of a single program.
 *
 * Many instances of an interpreter can be spawned for a
 * program. This is where the parsing of the programming language
 * will take place, and from which a new instance can be
 * instantiated to deal with a specific connection.
 */
class Interpreter {
public:
  /***
   * An interpreter is constructed from an already parsed program
   * tobe executed in the context of a given socket.
   */
  Interpreter(){};
};

} // namespace networkprotocoldsl

#endif // NETWORKPROTOCOLDSL_INTERPRETER_HPP
