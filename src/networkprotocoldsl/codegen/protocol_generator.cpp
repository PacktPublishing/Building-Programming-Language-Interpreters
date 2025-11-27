/**
 * protocol_generator - Command line tool for generating C++ code from
 *                      Network Protocol DSL specifications.
 *
 * Usage:
 *   protocol_generator <input.networkprotocoldsl> -n <namespace> -o <output_dir>
 *
 * Example:
 *   protocol_generator http.networkprotocoldsl -n myapp::http -o generated/
 */

#include <networkprotocoldsl/codegen/cppgenerator.hpp>
#include <networkprotocoldsl/lexer/tokenize.hpp>
#include <networkprotocoldsl/parser/parse.hpp>
#include <networkprotocoldsl/sema/analyze.hpp>

#include <CLI/CLI.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
  CLI::App app{"Generate C++ code from Network Protocol DSL specifications"};

  std::string input_file;
  std::string target_namespace;
  std::string output_dir = ".";
  std::string library_name;
  bool verbose = false;

  app.add_option("input", input_file,
                 "Input .networkprotocoldsl file to process")
      ->required()
      ->check(CLI::ExistingFile);

  app.add_option("-n,--namespace", target_namespace,
                 "Target C++ namespace (e.g., myapp::http)")
      ->required();

  app.add_option("-o,--output", output_dir,
                 "Output directory for generated files (default: current directory)");

  app.add_option("-l,--library", library_name,
                 "Name of the static library to generate (also generates CMakeLists.txt)");

  app.add_flag("-v,--verbose", verbose, "Enable verbose output");

  CLI11_PARSE(app, argc, argv);

  // Read the input file
  if (verbose) {
    std::cerr << "Reading: " << input_file << std::endl;
  }

  std::ifstream file(input_file);
  if (!file.is_open()) {
    std::cerr << "Error: Could not open input file: " << input_file << std::endl;
    return 1;
  }

  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  if (verbose) {
    std::cerr << "Read " << content.size() << " bytes" << std::endl;
  }

  // Tokenize
  if (verbose) {
    std::cerr << "Tokenizing..." << std::endl;
  }

  auto maybe_tokens = networkprotocoldsl::lexer::tokenize(content);
  if (!maybe_tokens.has_value()) {
    std::cerr << "Error: Failed to tokenize input file" << std::endl;
    return 1;
  }

  if (verbose) {
    std::cerr << "Tokenized " << maybe_tokens.value().size() << " tokens"
              << std::endl;
  }

  // Parse
  if (verbose) {
    std::cerr << "Parsing..." << std::endl;
  }

  auto parse_result = networkprotocoldsl::parser::parse(maybe_tokens.value());
  if (!parse_result.has_value()) {
    std::cerr << "Error: Failed to parse input file" << std::endl;
    return 1;
  }

  if (verbose) {
    std::cerr << "Parsed " << parse_result.value()->size() << " messages"
              << std::endl;
  }

  // Semantic analysis
  if (verbose) {
    std::cerr << "Analyzing..." << std::endl;
  }

  auto maybe_protocol =
      networkprotocoldsl::sema::analyze(parse_result.value());
  if (!maybe_protocol.has_value()) {
    std::cerr << "Error: Semantic analysis failed" << std::endl;
    return 1;
  }

  if (verbose) {
    std::cerr << "Analysis complete" << std::endl;
  }

  // Generate code
  if (verbose) {
    std::cerr << "Generating C++ code..." << std::endl;
    std::cerr << "  Namespace: " << target_namespace << std::endl;
    std::cerr << "  Output directory: " << output_dir << std::endl;
    if (!library_name.empty()) {
      std::cerr << "  Library name: " << library_name << std::endl;
    }
  }

  fs::path output_path(output_dir);
  networkprotocoldsl::codegen::CppGenerator generator(
      maybe_protocol.value(), target_namespace, output_path, library_name);

  if (!generator.generate()) {
    std::cerr << "Error: Code generation failed" << std::endl;
    for (const auto &error : generator.errors()) {
      std::cerr << "  " << error << std::endl;
    }
    return 1;
  }

  if (verbose) {
    std::cerr << "Code generation complete" << std::endl;
    std::cerr << "Generated files in: " << output_dir << std::endl;
  } else {
    std::cout << "Generated C++ code in: " << output_dir << std::endl;
  }

  return 0;
}
