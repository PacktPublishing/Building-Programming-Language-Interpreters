#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/interpreter.hpp>

#include <gtest/gtest.h>
#include <string>

TEST(starting_the_interpreter, empty_program) {
  std::string test_file = "data/002-empty-program.networkprotocoldsl";
  networkprotocoldsl::InterpretedProgram program(test_file);
  networkprotocoldsl::Interpreter interpreter = program.get_instance();
}
