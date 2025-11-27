/**
 * Test that verifies the generated code for the SMTP protocol compiles correctly.
 *
 * This test:
 * 1. Reads the SMTP protocol DSL file
 * 2. Generates C++ code using the codegen modules (including CMakeLists.txt)
 * 3. Writes the generated files to a temporary directory
 * 4. Uses CMake to configure and build the generated code
 *
 * If the generated code has syntax errors, the test will fail.
 */

#include <networkprotocoldsl/codegen/cppgenerator.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>
#include <networkprotocoldsl/parser/parse.hpp>
#include <networkprotocoldsl/sema/analyze.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

using namespace networkprotocoldsl;
using namespace networkprotocoldsl::codegen;

class CompileSMTPTest : public ::testing::Test {
protected:
  std::shared_ptr<const sema::ast::Protocol> protocol_;
  fs::path temp_dir_;
  fs::path build_dir_;
  fs::path smtp_file_;

  void SetUp() override {
    // Load and parse the SMTP protocol file
    smtp_file_ = fs::path(SMTP_SOURCE_DIR) / "smtp.networkprotocoldsl";
    ASSERT_TRUE(fs::exists(smtp_file_)) << "SMTP file not found: " << smtp_file_;

    std::ifstream file(smtp_file_);
    ASSERT_TRUE(file.is_open()) << "Could not open SMTP file";
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    // Tokenize
    auto maybe_tokens = lexer::tokenize(content);
    ASSERT_TRUE(maybe_tokens.has_value()) << "Failed to tokenize SMTP protocol";

    // Parse
    auto parse_result = parser::parse(maybe_tokens.value());
    ASSERT_TRUE(parse_result.has_value()) << "Failed to parse SMTP protocol";

    // Analyze
    auto maybe_protocol = sema::analyze(parse_result.value());
    ASSERT_TRUE(maybe_protocol.has_value())
        << "Failed semantic analysis of SMTP protocol";
    protocol_ = maybe_protocol.value();

    // Create temporary directory for generated files
    temp_dir_ = fs::temp_directory_path() / "codegen_test_smtp";
    build_dir_ = temp_dir_ / "build";
    fs::create_directories(temp_dir_);
    fs::create_directories(build_dir_);
  }

  void TearDown() override {
    // Clean up temporary directory
    if (fs::exists(temp_dir_)) {
      fs::remove_all(temp_dir_);
    }
  }

  // Helper to run a command and capture output
  std::pair<int, std::string> run_command(const std::string &cmd) {
    std::string full_cmd = cmd + " 2>&1";
    std::array<char, 128> buffer;
    std::string result;

    FILE *pipe = popen(full_cmd.c_str(), "r");
    if (!pipe) {
      return {-1, "Failed to run command"};
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
      result += buffer.data();
    }

    int status = pclose(pipe);
    int exit_code = WEXITSTATUS(status);

    return {exit_code, result};
  }
};

TEST_F(CompileSMTPTest, GeneratorSucceeds) {
  // Generate with library name to get CMakeLists.txt
  CppGenerator generator(protocol_, "smtp::generated", temp_dir_, "smtp_protocol");

  bool success = generator.generate();
  ASSERT_TRUE(success) << "Generator failed with errors: "
                       << (generator.errors().empty()
                               ? "unknown"
                               : generator.errors()[0]);

  // Verify all expected files were created
  EXPECT_TRUE(fs::exists(temp_dir_ / "protocol.hpp"));
  EXPECT_TRUE(fs::exists(temp_dir_ / "data_types.hpp"));
  EXPECT_TRUE(fs::exists(temp_dir_ / "data_types.cpp"));
  EXPECT_TRUE(fs::exists(temp_dir_ / "states.hpp"));
  EXPECT_TRUE(fs::exists(temp_dir_ / "states.cpp"));
  EXPECT_TRUE(fs::exists(temp_dir_ / "parser.hpp"));
  EXPECT_TRUE(fs::exists(temp_dir_ / "parser.cpp"));
  EXPECT_TRUE(fs::exists(temp_dir_ / "serializer.hpp"));
  EXPECT_TRUE(fs::exists(temp_dir_ / "serializer.cpp"));
  EXPECT_TRUE(fs::exists(temp_dir_ / "state_machine.hpp"));
  EXPECT_TRUE(fs::exists(temp_dir_ / "state_machine.cpp"));
  EXPECT_TRUE(fs::exists(temp_dir_ / "CMakeLists.txt"));
}

TEST_F(CompileSMTPTest, CMakeConfigures) {
  CppGenerator generator(protocol_, "smtp::generated", temp_dir_, "smtp_protocol");
  ASSERT_TRUE(generator.generate());

  // Run cmake to configure
  std::ostringstream cmd;
  cmd << "cmake -S " << temp_dir_.string() << " -B " << build_dir_.string();

  auto [exit_code, output] = run_command(cmd.str());
  EXPECT_EQ(exit_code, 0)
      << "CMake configuration failed:\n"
      << output;
}

TEST_F(CompileSMTPTest, CMakeBuilds) {
  CppGenerator generator(protocol_, "smtp::generated", temp_dir_, "smtp_protocol");
  ASSERT_TRUE(generator.generate());

  // Run cmake to configure
  std::ostringstream config_cmd;
  config_cmd << "cmake -S " << temp_dir_.string() << " -B " << build_dir_.string();
  auto [config_exit, config_output] = run_command(config_cmd.str());
  ASSERT_EQ(config_exit, 0)
      << "CMake configuration failed:\n"
      << config_output;

  // Run cmake to build
  std::ostringstream build_cmd;
  build_cmd << "cmake --build " << build_dir_.string();

  auto [build_exit, build_output] = run_command(build_cmd.str());
  EXPECT_EQ(build_exit, 0)
      << "CMake build failed:\n"
      << build_output;

  // Check that the library was created
  fs::path lib_path = build_dir_ / "libsmtp_protocol.a";
  EXPECT_TRUE(fs::exists(lib_path))
      << "Library was not created at " << lib_path.string();
}

TEST_F(CompileSMTPTest, GeneratedCodeCanBeUsed) {
  CppGenerator generator(protocol_, "smtp::generated", temp_dir_, "smtp_protocol");
  ASSERT_TRUE(generator.generate());

  // Create a simple test program that uses the generated code
  fs::path test_cpp = temp_dir_ / "test_usage.cpp";
  std::ofstream test_file(test_cpp);
  test_file << R"(
#include "protocol.hpp"

int main() {
    // Test that we can instantiate the types
    smtp::generated::SMTPServerGreetingData greeting;
    greeting.code_tens = 2;
    greeting.msg = "Hello";
    
    // Test state enum
    smtp::generated::State state = smtp::generated::State::Open;
    (void)state;
    
    // Test parser instantiation
    smtp::generated::SMTPServerGreetingParser parser;
    (void)parser;
    
    // Test serializer instantiation
    smtp::generated::SMTPServerGreetingSerializer serializer;
    (void)serializer;
    
    // Test state machine instantiation
    smtp::generated::ClientStateMachine client_sm;
    smtp::generated::ServerStateMachine server_sm;
    (void)client_sm;
    (void)server_sm;
    
    return 0;
}
)";
  test_file.close();

  // Append a test executable to CMakeLists.txt
  std::ofstream cmake_append(temp_dir_ / "CMakeLists.txt", std::ios::app);
  cmake_append << R"(

# Test executable
add_executable(test_usage test_usage.cpp)
target_link_libraries(test_usage PRIVATE smtp_protocol)
)";
  cmake_append.close();

  // Configure and build
  std::ostringstream config_cmd;
  config_cmd << "cmake -S " << temp_dir_.string() << " -B " << build_dir_.string();
  auto [config_exit, config_output] = run_command(config_cmd.str());
  ASSERT_EQ(config_exit, 0)
      << "CMake configuration failed:\n"
      << config_output;

  std::ostringstream build_cmd;
  build_cmd << "cmake --build " << build_dir_.string();
  auto [build_exit, build_output] = run_command(build_cmd.str());
  EXPECT_EQ(build_exit, 0)
      << "CMake build failed:\n"
      << build_output;

  // Run the test program
  if (build_exit == 0) {
    fs::path test_exe = build_dir_ / "test_usage";
    auto [run_exit, run_output] = run_command(test_exe.string());
    EXPECT_EQ(run_exit, 0) << "Test program failed:\n" << run_output;
  }
}

TEST_F(CompileSMTPTest, GeneratedDataTypesHaveExpectedStructure) {
  CppGenerator generator(protocol_, "smtp::generated", temp_dir_, "smtp_protocol");
  ASSERT_TRUE(generator.generate());

  // Read the data_types.hpp and verify it has expected content
  std::ifstream file(temp_dir_ / "data_types.hpp");
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());

  // Check for expected structs based on SMTP protocol
  EXPECT_TRUE(content.find("SMTPServerGreetingData") != std::string::npos)
      << "Missing SMTPServerGreetingData struct";
  EXPECT_TRUE(content.find("SMTPEHLOCommandData") != std::string::npos)
      << "Missing SMTPEHLOCommandData struct";
  EXPECT_TRUE(content.find("namespace smtp::generated") != std::string::npos)
      << "Wrong namespace in data_types.hpp";
}

TEST_F(CompileSMTPTest, GeneratedStatesHaveExpectedStructure) {
  CppGenerator generator(protocol_, "smtp::generated", temp_dir_, "smtp_protocol");
  ASSERT_TRUE(generator.generate());

  // Read the states.hpp and verify it has expected content
  std::ifstream file(temp_dir_ / "states.hpp");
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());

  // Check for state enum
  EXPECT_TRUE(content.find("enum class State") != std::string::npos)
      << "Missing State enum";
  EXPECT_TRUE(content.find("Open") != std::string::npos)
      << "Missing Open state";
  EXPECT_TRUE(content.find("Closed") != std::string::npos)
      << "Missing Closed state";
}

TEST_F(CompileSMTPTest, GeneratedCMakeListsHasCorrectContent) {
  CppGenerator generator(protocol_, "smtp::generated", temp_dir_, "smtp_protocol");
  ASSERT_TRUE(generator.generate());

  // Read the CMakeLists.txt and verify it has expected content
  std::ifstream file(temp_dir_ / "CMakeLists.txt");
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());

  EXPECT_TRUE(content.find("cmake_minimum_required") != std::string::npos)
      << "Missing cmake_minimum_required";
  EXPECT_TRUE(content.find("add_library(smtp_protocol STATIC") != std::string::npos)
      << "Missing add_library for smtp_protocol";
  EXPECT_TRUE(content.find("cxx_std_20") != std::string::npos)
      << "Missing C++20 standard requirement";
  EXPECT_TRUE(content.find("target_include_directories") != std::string::npos)
      << "Missing target_include_directories";
}
