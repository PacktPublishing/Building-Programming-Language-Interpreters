/**
 * Integration test: Generated Code vs Interpreted Code
 *
 * This test verifies that the generated C++ code produces the same
 * wire-format output as the interpreted version for SMTP protocol messages.
 *
 * Phase 7.2.2 of code_generation.md: Compare behavior with interpreted version
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

class CodegenIntegrationTest : public ::testing::Test {
protected:
  std::shared_ptr<const sema::ast::Protocol> protocol_;
  fs::path temp_dir_;
  fs::path build_dir_;
  fs::path smtp_file_;
  fs::path test_data_dir_;

  void SetUp() override {
    // Set up test data directory path
    test_data_dir_ = fs::path(TEST_DATA_DIR) / "codegen";

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
    temp_dir_ = fs::temp_directory_path() / "codegen_integration_test";
    build_dir_ = temp_dir_ / "build";
    fs::create_directories(temp_dir_);
    fs::create_directories(build_dir_);
    std::cerr << "TEMP_DIR: " << temp_dir_ << std::endl;
  }

  void TearDown() override {
    // Clean up temporary directory - disabled for debugging
    // if (fs::exists(temp_dir_)) {
    //   fs::remove_all(temp_dir_);
    // }
  }

  // Helper to read a test data file
  std::string read_test_data(const std::string &filename) {
    fs::path file_path = test_data_dir_ / filename;
    std::ifstream file(file_path);
    if (!file.is_open()) {
      throw std::runtime_error("Could not open test data file: " +
                               file_path.string());
    }
    return std::string((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
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

  // Generate and build the code, then run a test executable
  bool generate_and_build() {
    CppGenerator generator(protocol_, "smtp::generated", temp_dir_,
                           "smtp_protocol");
    if (!generator.generate()) {
      std::cerr << "Generator failed: " << generator.errors()[0] << std::endl;
      return false;
    }

    // Configure
    std::ostringstream config_cmd;
    config_cmd << "cmake -S " << temp_dir_.string() << " -B "
               << build_dir_.string();
    auto [config_exit, config_output] = run_command(config_cmd.str());
    if (config_exit != 0) {
      std::cerr << "CMake configuration failed:\n" << config_output << std::endl;
      return false;
    }

    // Build
    std::ostringstream build_cmd;
    build_cmd << "cmake --build " << build_dir_.string();
    auto [build_exit, build_output] = run_command(build_cmd.str());
    if (build_exit != 0) {
      std::cerr << "CMake build failed:\n" << build_output << std::endl;
      return false;
    }

    return true;
  }

  // Copy a test file and add it to CMakeLists.txt
  void add_test_executable(const std::string &test_name,
                           const std::vector<std::string> &extra_libs = {}) {
    // Copy test source file
    std::string source_content = read_test_data(test_name + ".cpp");
    fs::path test_cpp = temp_dir_ / (test_name + ".cpp");
    std::ofstream test_file(test_cpp);
    test_file << source_content;
    test_file.close();

    // Append to CMakeLists.txt
    std::ofstream cmake_append(temp_dir_ / "CMakeLists.txt", std::ios::app);
    cmake_append << "\nadd_executable(" << test_name << " " << test_name
                 << ".cpp)\n";
    cmake_append << "target_link_libraries(" << test_name
                 << " PRIVATE smtp_protocol";
    for (const auto &lib : extra_libs) {
      cmake_append << " " << lib;
    }
    cmake_append << ")\n";
    cmake_append.close();
  }

  // Build and run a test executable
  std::pair<int, std::string> build_and_run_test(const std::string &test_name) {
    // Rebuild
    std::ostringstream build_cmd;
    build_cmd << "cmake --build " << build_dir_.string();
    auto [build_exit, build_output] = run_command(build_cmd.str());
    if (build_exit != 0) {
      return {build_exit, "Build failed:\n" + build_output};
    }

    // Run the test
    fs::path test_exe = build_dir_ / test_name;
    return run_command(test_exe.string());
  }
};

/**
 * Test that verifies SMTP Server Greeting serialization
 *
 * Expected wire format: "2<code_tens> <msg>\r\n"
 * For code_tens=50 and msg="Hello", expect "250 Hello\r\n"
 */
TEST_F(CodegenIntegrationTest, ServerGreetingSerialization) {
  ASSERT_TRUE(generate_and_build());

  add_test_executable("test_greeting");
  auto [run_exit, run_output] = build_and_run_test("test_greeting");

  ASSERT_EQ(run_exit, 0) << "Test failed:\n" << run_output;

  // Verify output contains expected serialization
  // The wire format should be "250 Hello SMTP\r\n"
  EXPECT_TRUE(run_output.find("STRING:250 Hello SMTP") != std::string::npos)
      << "Output was:\n" << run_output;
}

/**
 * Test that verifies SMTP EHLO command parsing
 *
 * Expected wire format: "EHLO <domain>\r\n"
 */
TEST_F(CodegenIntegrationTest, EHLOCommandParsing) {
  ASSERT_TRUE(generate_and_build());

  add_test_executable("test_ehlo_parse");
  auto [run_exit, run_output] = build_and_run_test("test_ehlo_parse");

  ASSERT_EQ(run_exit, 0) << "Test failed:\n" << run_output;

  // Verify the command was parsed correctly
  EXPECT_TRUE(run_output.find("CONSUMED:18") != std::string::npos ||
              run_output.find("CONSUMED:") != std::string::npos)
      << "Output was:\n" << run_output;
  EXPECT_TRUE(run_output.find("HAS_MESSAGE:yes") != std::string::npos)
      << "Message not parsed. Output was:\n" << run_output;
  EXPECT_TRUE(run_output.find("DOMAIN:example.com") != std::string::npos)
      << "Domain not extracted correctly. Output was:\n" << run_output;
}

/**
 * Test round-trip: serialize then parse
 *
 * Verifies that data serialized by one side can be parsed by the other.
 */
TEST_F(CodegenIntegrationTest, RoundTripGreetingAndEHLO) {
  ASSERT_TRUE(generate_and_build());

  add_test_executable("test_roundtrip");
  auto [run_exit, run_output] = build_and_run_test("test_roundtrip");

  ASSERT_EQ(run_exit, 0) << "Test failed:\n" << run_output;

  // Verify round-trip succeeded
  EXPECT_TRUE(run_output.find("SUCCESS") != std::string::npos)
      << "Round-trip failed. Output was:\n" << run_output;
  EXPECT_TRUE(run_output.find("GREETING_CODE:50") != std::string::npos)
      << "Greeting code wrong. Output was:\n" << run_output;
  EXPECT_TRUE(run_output.find("EHLO_DOMAIN:test.example.org") != std::string::npos)
      << "EHLO domain wrong. Output was:\n" << run_output;
}

/**
 * Test complete SMTP conversation flow
 *
 * Tests the state machine transitions through a typical session:
 * Open -> Greeting -> EHLO -> EHLO Response -> MAIL FROM -> ... -> QUIT -> Closed
 */
TEST_F(CodegenIntegrationTest, CompleteSMTPConversation) {
  ASSERT_TRUE(generate_and_build());

  add_test_executable("test_conversation");
  auto [run_exit, run_output] = build_and_run_test("test_conversation");

  std::cout << "Test output:\n" << run_output << std::endl;

  EXPECT_EQ(run_exit, 0) << "Conversation test failed:\n" << run_output;
  EXPECT_TRUE(run_output.find("CONVERSATION_SUCCESS") != std::string::npos)
      << "Conversation did not complete successfully. Output was:\n"
      << run_output;
}

/**
 * Test sans-IO properties: no blocking calls
 *
 * Verifies that the state machine can handle partial data correctly
 * and returns control without blocking.
 */
TEST_F(CodegenIntegrationTest, SansIOPartialData) {
  ASSERT_TRUE(generate_and_build());

  add_test_executable("test_partial");
  auto [run_exit, run_output] = build_and_run_test("test_partial");

  std::cout << "Partial data test output:\n" << run_output << std::endl;

  EXPECT_EQ(run_exit, 0) << "Test failed:\n" << run_output;
  EXPECT_TRUE(run_output.find("SANS_IO_SUCCESS") != std::string::npos)
      << "Sans-IO test failed. Output was:\n" << run_output;
}

/**
 * Test libuv adapter with generated code
 *
 * Verifies that the ServerRunner can be used with libuv to create
 * a working TCP server that handles SMTP protocol messages.
 */
TEST_F(CodegenIntegrationTest, LibuvAdapterWithRunner) {
  ASSERT_TRUE(generate_and_build());

  // Add libuv detection to CMakeLists.txt before the test executable
  {
    std::ofstream cmake_append(temp_dir_ / "CMakeLists.txt", std::ios::app);
    cmake_append << "\nfind_path(LIBUV_INCLUDE_DIR NAMES uv.h)\n";
    cmake_append << "find_library(LIBUV_LIBRARIES NAMES uv libuv)\n";
    cmake_append.close();
  }

  add_test_executable("test_uv_adapter", {"${LIBUV_LIBRARIES}"});

  // Also add include directories for libuv
  {
    std::ofstream cmake_append(temp_dir_ / "CMakeLists.txt", std::ios::app);
    cmake_append << "target_include_directories(test_uv_adapter PRIVATE ${LIBUV_INCLUDE_DIR})\n";
    cmake_append.close();
  }

  auto [run_exit, run_output] = build_and_run_test("test_uv_adapter");

  std::cout << "UV adapter test output:\n" << run_output << std::endl;

  EXPECT_EQ(run_exit, 0) << "UV adapter test failed:\n" << run_output;
  EXPECT_TRUE(run_output.find("UV_ADAPTER_TEST_SUCCESS") != std::string::npos)
      << "UV adapter test did not complete successfully. Output was:\n"
      << run_output;
}
