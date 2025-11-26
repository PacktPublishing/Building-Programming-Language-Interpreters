#include <networkprotocoldsl/codegen/cppgenerator.hpp>
#include <networkprotocoldsl/codegen/generate_data_types.hpp>
#include <networkprotocoldsl/codegen/generate_parser.hpp>
#include <networkprotocoldsl/codegen/generate_runner.hpp>
#include <networkprotocoldsl/codegen/generate_serializer.hpp>
#include <networkprotocoldsl/codegen/generate_state_machine.hpp>
#include <networkprotocoldsl/codegen/generate_states.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace networkprotocoldsl::codegen {

CppGenerator::CppGenerator(std::shared_ptr<const sema::ast::Protocol> protocol,
                           std::string target_namespace,
                           std::filesystem::path target_directory,
                           std::string library_name)
    : ctx_(std::move(target_namespace)),
      info_(std::move(protocol)),
      target_directory_(std::move(target_directory)),
      library_name_(std::move(library_name)) {}

bool CppGenerator::generate() {
  errors_.clear();

  // Create target directory if it doesn't exist
  std::error_code ec;
  std::filesystem::create_directories(target_directory_, ec);
  if (ec) {
    errors_.push_back("Failed to create directory: " +
                      target_directory_.string() + " (" + ec.message() + ")");
    return false;
  }

  bool success = true;

  // Generate data types
  auto data_types_result = generate_data_types(ctx_, info_);
  errors_.insert(errors_.end(), data_types_result.errors.begin(),
                 data_types_result.errors.end());
  success = write_pair("data_types", data_types_result.header,
                       data_types_result.source) &&
            success;

  // Generate states
  auto states_result = generate_states(ctx_, info_);
  errors_.insert(errors_.end(), states_result.errors.begin(),
                 states_result.errors.end());
  success =
      write_pair("states", states_result.header, states_result.source) &&
      success;

  // Generate parser
  auto parser_result = generate_parser(ctx_, info_);
  errors_.insert(errors_.end(), parser_result.errors.begin(),
                 parser_result.errors.end());
  success =
      write_pair("parser", parser_result.header, parser_result.source) &&
      success;

  // Generate serializer
  auto serializer_result = generate_serializer(ctx_, info_);
  errors_.insert(errors_.end(), serializer_result.errors.begin(),
                 serializer_result.errors.end());
  success = write_pair("serializer", serializer_result.header,
                       serializer_result.source) &&
            success;

  // Generate state machine
  auto state_machine_result = generate_state_machine(ctx_, info_);
  errors_.insert(errors_.end(), state_machine_result.errors.begin(),
                 state_machine_result.errors.end());
  success = write_pair("state_machine", state_machine_result.header,
                       state_machine_result.source) &&
            success;

  // Generate runner (callback-based wrapper)
  auto runner_result = generate_runner(ctx_, info_);
  errors_.insert(errors_.end(), runner_result.errors.begin(),
                 runner_result.errors.end());
  success = write_pair("runner", runner_result.header,
                       runner_result.source) &&
            success;

  // Generate main header
  success = write_file("protocol.hpp", generate_main_header()) && success;

  // Generate CMakeLists.txt if library name is specified
  if (!library_name_.empty()) {
    success = write_file("CMakeLists.txt", generate_cmake_lists()) && success;
  }

  return success;
}

bool CppGenerator::write_file(const std::string &filename,
                              const std::string &content) {
  auto path = target_directory_ / filename;
  std::ofstream file(path);
  if (!file) {
    errors_.push_back("Failed to open file for writing: " + path.string());
    return false;
  }
  file << content;
  if (!file) {
    errors_.push_back("Failed to write to file: " + path.string());
    return false;
  }
  return true;
}

bool CppGenerator::write_pair(const std::string &basename,
                              const std::string &header,
                              const std::string &source) {
  bool success = true;
  success = write_file(basename + ".hpp", header) && success;
  success = write_file(basename + ".cpp", source) && success;
  return success;
}

std::string CppGenerator::generate_main_header() {
  std::ostringstream oss;
  std::string guard = ctx_.header_guard("protocol.hpp");

  oss << "// This file is auto-generated. Do not edit.\n\n";
  oss << "#ifndef " << guard << "\n";
  oss << "#define " << guard << "\n";
  oss << "\n";
  oss << "// Main protocol header - includes all generated components\n";
  oss << "\n";
  oss << "#include \"data_types.hpp\"\n";
  oss << "#include \"states.hpp\"\n";
  oss << "#include \"parser.hpp\"\n";
  oss << "#include \"serializer.hpp\"\n";
  oss << "#include \"state_machine.hpp\"\n";
  oss << "#include \"runner.hpp\"\n";
  oss << "\n";
  oss << "#endif // " << guard << "\n";

  return oss.str();
}

std::string CppGenerator::generate_cmake_lists() {
  std::ostringstream oss;

  oss << "# This file is auto-generated. Do not edit.\n";
  oss << "# CMakeLists.txt for " << library_name_ << "\n";
  oss << "\n";
  oss << "cmake_minimum_required(VERSION 3.14)\n";
  oss << "\n";
  oss << "# Define the static library\n";
  oss << "add_library(" << library_name_ << " STATIC\n";
  oss << "    data_types.cpp\n";
  oss << "    states.cpp\n";
  oss << "    parser.cpp\n";
  oss << "    serializer.cpp\n";
  oss << "    state_machine.cpp\n";
  oss << "    runner.cpp\n";
  oss << ")\n";
  oss << "\n";
  oss << "# Set C++ standard (C++20 for concepts support)\n";
  oss << "target_compile_features(" << library_name_ << " PUBLIC cxx_std_20)\n";
  oss << "\n";
  oss << "# Include current directory for headers\n";
  oss << "target_include_directories(" << library_name_ << " PUBLIC\n";
  oss << "    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>\n";
  oss << "    $<INSTALL_INTERFACE:include>\n";
  oss << ")\n";

  return oss.str();
}

} // namespace networkprotocoldsl::codegen
