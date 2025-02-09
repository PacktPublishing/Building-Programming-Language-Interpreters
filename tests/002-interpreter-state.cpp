#include <networkprotocoldsl/interpretedprogram.hpp>
#include <networkprotocoldsl/interpreter.hpp>

#include <gtest/gtest.h>
#include <string>

TEST(starting_the_interpreter, empty_program) {
  std::string test_file =
      std::string(TEST_DATA_DIR) + "/002-empty-program.networkprotocoldsl";
  auto maybe_program =
      networkprotocoldsl::InterpretedProgram::generate_client(test_file);
  ASSERT_TRUE(maybe_program.has_value());
  auto program = maybe_program.value();
  networkprotocoldsl::Interpreter interpreter = program.get_instance();
}
