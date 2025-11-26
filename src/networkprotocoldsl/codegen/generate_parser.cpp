#include <networkprotocoldsl/codegen/generate_parser.hpp>
#include <networkprotocoldsl/codegen/typemapping.hpp>

#include <map>
#include <set>
#include <sstream>

namespace networkprotocoldsl::codegen {

namespace {

// Generate the header declaration for a single message parser
void generate_message_parser_header(std::ostringstream &header,
                                    const ReadTransitionInfo &rt) {
  header << "// Parser for message: " << rt.message_name << "\n";
  header << "class " << rt.identifier << "Parser {\n";
  header << "public:\n";
  header << "    " << rt.identifier << "Parser() = default;\n";
  header << "    \n";
  header << "    // Reset the parser to initial state\n";
  header << "    void reset();\n";
  header << "    \n";
  header << "    // Parse input bytes, returns result with status and bytes consumed\n";
  header << "    ParseResult parse(std::string_view input);\n";
  header << "    \n";
  header << "    // Check if parsing is complete\n";
  header << "    bool is_complete() const { return complete_; }\n";
  header << "    \n";
  header << "    // Take the parsed data (only valid when complete)\n";
  header << "    " << rt.identifier << "Data take_data();\n";
  header << "\n";
  header << "private:\n";
  header << "    bool complete_ = false;\n";
  header << "    size_t stage_ = 0;\n";

  // Add buffer fields for fields that need accumulation
  for (const auto &action : rt.actions) {
    std::visit(
        [&](const auto &a) {
          using T = std::decay_t<decltype(*a)>;
          if constexpr (std::is_same_v<T,
                                       sema::ast::action::ReadOctetsUntilTerminator>) {
            if (a->identifier) {
              header << "    std::string " << a->identifier->name
                     << "_buffer_;\n";
            }
          } else if constexpr (std::is_same_v<T, sema::ast::action::Loop>) {
            // Add vector for loop collection and loop parsing state
            if (a->collection) {
              header << "    std::vector<" << rt.identifier << "_" 
                     << a->collection->name << "Element> " 
                     << a->collection->name << "_buffer_;\n";
              header << "    size_t loop_" << a->collection->name << "_stage_ = 0;\n";
              // Add element field buffers for nested actions
              for (const auto &inner_action : a->actions) {
                std::visit(
                    [&](const auto &inner_a) {
                      using InnerT = std::decay_t<decltype(*inner_a)>;
                      if constexpr (std::is_same_v<InnerT,
                                       sema::ast::action::ReadOctetsUntilTerminator>) {
                        if (inner_a->identifier) {
                          header << "    std::string loop_" << a->collection->name 
                                 << "_" << inner_a->identifier->name << "_buffer_;\n";
                        }
                      }
                    },
                    inner_action);
              }
            }
          }
        },
        action);
  }
  header << "};\n\n";
}

// Generate the reset method for a message parser
void generate_message_parser_reset(std::ostringstream &source,
                                   const ReadTransitionInfo &rt) {
  source << "void " << rt.identifier << "Parser::reset() {\n";
  source << "    complete_ = false;\n";
  source << "    stage_ = 0;\n";
  for (const auto &action : rt.actions) {
    std::visit(
        [&](const auto &a) {
          using T = std::decay_t<decltype(*a)>;
          if constexpr (std::is_same_v<T,
                                       sema::ast::action::ReadOctetsUntilTerminator>) {
            if (a->identifier) {
              source << "    " << a->identifier->name << "_buffer_.clear();\n";
            }
          } else if constexpr (std::is_same_v<T, sema::ast::action::Loop>) {
            if (a->collection) {
              source << "    " << a->collection->name << "_buffer_.clear();\n";
              source << "    loop_" << a->collection->name << "_stage_ = 0;\n";
              // Clear inner action buffers
              for (const auto &inner_action : a->actions) {
                std::visit(
                    [&](const auto &inner_a) {
                      using InnerT = std::decay_t<decltype(*inner_a)>;
                      if constexpr (std::is_same_v<InnerT,
                                       sema::ast::action::ReadOctetsUntilTerminator>) {
                        if (inner_a->identifier) {
                          source << "    loop_" << a->collection->name << "_" 
                                 << inner_a->identifier->name << "_buffer_.clear();\n";
                        }
                      }
                    },
                    inner_action);
              }
            }
          }
        },
        action);
  }
  source << "}\n\n";
}

// Generate the parse method for a message parser
void generate_message_parser_parse(std::ostringstream &source,
                                   const ReadTransitionInfo &rt) {
  source << "ParseResult " << rt.identifier
         << "Parser::parse(std::string_view input) {\n";
  source << "    size_t total_consumed = 0;\n";
  source << "    \n";
  source << "    while (!complete_ && !input.empty()) {\n";
  source << "        switch (stage_) {\n";

  size_t stage = 0;
  for (const auto &action : rt.actions) {
    source << "        case " << stage << ": {\n";

    std::visit(
        [&](const auto &a) {
          using T = std::decay_t<decltype(*a)>;

          if constexpr (std::is_same_v<T, sema::ast::action::ReadStaticOctets>) {
            // Match static octets
            std::string escaped =
                OutputContext::escape_string_literal(a->octets);
            source << "            // Match static octets: " << escaped << "\n";
            source << "            constexpr size_t len = " << a->octets.size()
                   << ";\n";
            source << "            if (input.size() < len) {\n";
            source << "                return {ParseStatus::NeedMoreData, "
                      "total_consumed};\n";
            source << "            }\n";
            source << "            static const char expected[] = " << escaped
                   << ";\n";
            source << "            if (std::memcmp(input.data(), expected, "
                      "len) != 0) {\n";
            source << "                return {ParseStatus::Error, "
                      "total_consumed};\n";
            source << "            }\n";
            source << "            input.remove_prefix(len);\n";
            source << "            total_consumed += len;\n";
            source << "            ++stage_;\n";
            source << "            break;\n";
          } else if constexpr (std::is_same_v<
                                   T, sema::ast::action::ReadOctetsUntilTerminator>) {
            // Read until terminator
            std::string escaped =
                OutputContext::escape_string_literal(a->terminator);
            source << "            // Read until terminator: " << escaped
                   << "\n";
            source << "            {\n";
            source << "                constexpr size_t term_len = "
                   << a->terminator.size() << ";\n";
            source << "                static const char terminator[] = "
                   << escaped << ";\n";
            source << "                \n";
            source << "                // Search for terminator in input\n";
            source << "                size_t pos = 0;\n";
            source << "                bool found = false;\n";
            source << "                while (pos + term_len <= input.size()) "
                      "{\n";
            source << "                    if (std::memcmp(input.data() + pos, "
                      "terminator, term_len) == 0) {\n";
            source << "                        found = true;\n";
            source << "                        break;\n";
            source << "                    }\n";
            source << "                    ++pos;\n";
            source << "                }\n";
            source << "                \n";
            source << "                if (found) {\n";
            if (a->identifier) {
              source << "                    " << a->identifier->name
                     << "_buffer_.append(input.data(), pos);\n";
            }
            source << "                    input.remove_prefix(pos + "
                      "term_len);\n";
            source << "                    total_consumed += pos + term_len;\n";
            source << "                    ++stage_;\n";
            source << "                } else {\n";
            source << "                    // Buffer what we have and wait for "
                      "more\n";
            if (a->identifier) {
              source << "                    " << a->identifier->name
                     << "_buffer_.append(input.data(), input.size());\n";
            }
            source << "                    total_consumed += input.size();\n";
            source << "                    return {ParseStatus::NeedMoreData, "
                      "total_consumed};\n";
            source << "                }\n";
            source << "            }\n";
            source << "            break;\n";
          } else if constexpr (std::is_same_v<T, sema::ast::action::Loop>) {
            // Handle loop parsing
            std::string collection_name = a->collection ? a->collection->name : "items";
            std::string term_escaped = OutputContext::escape_string_literal(a->terminator);
            
            source << "            // Loop parsing for collection: " << collection_name << "\n";
            source << "            {\n";
            source << "                constexpr size_t term_len = " << a->terminator.size() << ";\n";
            source << "                static const char loop_terminator[] = " << term_escaped << ";\n";
            source << "                \n";
            source << "                // Check if we've hit the loop terminator\n";
            source << "                if (input.size() >= term_len && \n";
            source << "                    std::memcmp(input.data(), loop_terminator, term_len) == 0) {\n";
            source << "                    // End of loop\n";
            source << "                    input.remove_prefix(term_len);\n";
            source << "                    total_consumed += term_len;\n";
            source << "                    ++stage_;\n";
            source << "                    break;\n";
            source << "                }\n";
            source << "                \n";
            source << "                // Need at least some data to parse element\n";
            source << "                if (input.empty()) {\n";
            source << "                    return {ParseStatus::NeedMoreData, total_consumed};\n";
            source << "                }\n";
            source << "                \n";
            source << "                // Parse one element\n";
            source << "                switch (loop_" << collection_name << "_stage_) {\n";
            
            size_t inner_stage = 0;
            for (const auto &inner_action : a->actions) {
              source << "                case " << inner_stage << ": {\n";
              
              std::visit(
                  [&](const auto &inner_a) {
                    using InnerT = std::decay_t<decltype(*inner_a)>;
                    
                    if constexpr (std::is_same_v<InnerT, sema::ast::action::ReadStaticOctets>) {
                      std::string escaped = OutputContext::escape_string_literal(inner_a->octets);
                      source << "                    // Match static octets: " << escaped << "\n";
                      source << "                    constexpr size_t len = " << inner_a->octets.size() << ";\n";
                      source << "                    if (input.size() < len) {\n";
                      source << "                        return {ParseStatus::NeedMoreData, total_consumed};\n";
                      source << "                    }\n";
                      source << "                    static const char expected[] = " << escaped << ";\n";
                      source << "                    if (std::memcmp(input.data(), expected, len) != 0) {\n";
                      source << "                        return {ParseStatus::Error, total_consumed};\n";
                      source << "                    }\n";
                      source << "                    input.remove_prefix(len);\n";
                      source << "                    total_consumed += len;\n";
                      source << "                    ++loop_" << collection_name << "_stage_;\n";
                      source << "                    break;\n";
                    } else if constexpr (std::is_same_v<InnerT, sema::ast::action::ReadOctetsUntilTerminator>) {
                      std::string escaped = OutputContext::escape_string_literal(inner_a->terminator);
                      std::string buffer_name = "loop_" + collection_name + "_" + 
                                                (inner_a->identifier ? inner_a->identifier->name : "data") + "_buffer_";
                      source << "                    // Read until terminator: " << escaped << "\n";
                      source << "                    {\n";
                      source << "                        constexpr size_t elem_term_len = " << inner_a->terminator.size() << ";\n";
                      source << "                        static const char elem_terminator[] = " << escaped << ";\n";
                      source << "                        size_t pos = 0;\n";
                      source << "                        bool found = false;\n";
                      source << "                        while (pos + elem_term_len <= input.size()) {\n";
                      source << "                            if (std::memcmp(input.data() + pos, elem_terminator, elem_term_len) == 0) {\n";
                      source << "                                found = true;\n";
                      source << "                                break;\n";
                      source << "                            }\n";
                      source << "                            ++pos;\n";
                      source << "                        }\n";
                      source << "                        if (found) {\n";
                      if (inner_a->identifier) {
                        source << "                            " << buffer_name << ".append(input.data(), pos);\n";
                      }
                      source << "                            input.remove_prefix(pos + elem_term_len);\n";
                      source << "                            total_consumed += pos + elem_term_len;\n";
                      source << "                            ++loop_" << collection_name << "_stage_;\n";
                      source << "                        } else {\n";
                      if (inner_a->identifier) {
                        source << "                            " << buffer_name << ".append(input.data(), input.size());\n";
                      }
                      source << "                            total_consumed += input.size();\n";
                      source << "                            return {ParseStatus::NeedMoreData, total_consumed};\n";
                      source << "                        }\n";
                      source << "                    }\n";
                      source << "                    break;\n";
                    }
                  },
                  inner_action);
              
              source << "                }\n";
              ++inner_stage;
            }
            
            // After all inner stages, create element and add to collection
            source << "                default: {\n";
            source << "                    // Element complete, add to collection\n";
            source << "                    " << rt.identifier << "_" << collection_name << "Element elem;\n";
            
            // Move buffers into element
            for (const auto &inner_action : a->actions) {
              std::visit(
                  [&](const auto &inner_a) {
                    using InnerT = std::decay_t<decltype(*inner_a)>;
                    if constexpr (std::is_same_v<InnerT, sema::ast::action::ReadOctetsUntilTerminator>) {
                      if (inner_a->identifier) {
                        std::string buffer_name = "loop_" + collection_name + "_" + inner_a->identifier->name + "_buffer_";
                        source << "                    elem." << inner_a->identifier->name 
                               << " = std::move(" << buffer_name << ");\n";
                        source << "                    " << buffer_name << ".clear();\n";
                      }
                    }
                  },
                  inner_action);
            }
            
            source << "                    " << collection_name << "_buffer_.push_back(std::move(elem));\n";
            source << "                    loop_" << collection_name << "_stage_ = 0;\n";
            source << "                    break;\n";
            source << "                }\n";
            source << "                }\n";  // End inner switch
            source << "            }\n";  // End loop block
            source << "            break;\n";
          }
        },
        action);

    source << "        }\n";
    ++stage;
  }

  // Final stage - mark complete
  source << "        default:\n";
  source << "            complete_ = true;\n";
  source << "            break;\n";
  source << "        }\n";
  source << "    }\n";
  source << "    \n";
  source << "    return {complete_ ? ParseStatus::Complete : "
            "ParseStatus::NeedMoreData, total_consumed};\n";
  source << "}\n\n";
}

// Helper to get lookahead prefix for a read transition
// Returns the static octets prefix if the first action is ReadStaticOctets, empty otherwise
std::string get_lookahead_prefix(const ReadTransitionInfo &rt) {
  if (rt.actions.empty()) {
    return "";
  }
  
  const auto &first_action = rt.actions[0];
  if (auto *static_octets = std::get_if<std::shared_ptr<const sema::ast::action::ReadStaticOctets>>(&first_action)) {
    return (*static_octets)->octets;
  }
  
  return "";
}

// Helper to get C++ type from MessageData for a field
std::string get_field_cpp_type(const ReadTransitionInfo &rt,
                               const std::string &field_name) {
  if (!rt.data) {
    return "std::string"; // default fallback
  }
  auto it = rt.data->find(field_name);
  if (it == rt.data->end()) {
    return "std::string"; // default fallback
  }
  auto mapping = type_to_cpp(it->second, "");
  if (!mapping) {
    return "std::string"; // default fallback
  }
  return mapping->cpp_type;
}

// Helper to check if a type is a numeric type that needs conversion from string
bool is_numeric_type(const std::string &cpp_type) {
  return cpp_type == "uint8_t" || cpp_type == "int8_t" ||
         cpp_type == "uint16_t" || cpp_type == "int16_t" ||
         cpp_type == "uint32_t" || cpp_type == "int32_t" ||
         cpp_type == "uint64_t" || cpp_type == "int64_t";
}

// Generate the take_data method for a message parser
void generate_message_parser_take_data(std::ostringstream &source,
                                       const ReadTransitionInfo &rt) {
  source << rt.identifier << "Data " << rt.identifier
         << "Parser::take_data() {\n";
  source << "    " << rt.identifier << "Data data;\n";
  for (const auto &action : rt.actions) {
    std::visit(
        [&](const auto &a) {
          using T = std::decay_t<decltype(*a)>;
          if constexpr (std::is_same_v<T,
                                       sema::ast::action::ReadOctetsUntilTerminator>) {
            if (a->identifier) {
              const std::string &field_name = a->identifier->name;
              std::string cpp_type = get_field_cpp_type(rt, field_name);

              if (is_numeric_type(cpp_type)) {
                // Convert from ASCII string to numeric value
                // For AsciiInt encoding, each character is a digit
                source << "    if (!" << field_name << "_buffer_.empty()) {\n";
                if (cpp_type.find("uint") == 0) {
                  source << "        data." << field_name << " = static_cast<"
                         << cpp_type << ">(std::stoul(" << field_name
                         << "_buffer_));\n";
                } else {
                  source << "        data." << field_name << " = static_cast<"
                         << cpp_type << ">(std::stol(" << field_name
                         << "_buffer_));\n";
                }
                source << "    } else {\n";
                source << "        data." << field_name << " = 0;\n";
                source << "    }\n";
              } else {
                // String type - just move
                source << "    data." << field_name << " = std::move("
                       << field_name << "_buffer_);\n";
              }
            }
          } else if constexpr (std::is_same_v<T, sema::ast::action::Loop>) {
            if (a->collection) {
              // Move the collection buffer to data
              source << "    data." << a->collection->name << " = std::move("
                     << a->collection->name << "_buffer_);\n";
            }
          }
        },
        action);
  }
  source << "    reset();\n";
  source << "    return data;\n";
  source << "}\n\n";
}

} // anonymous namespace

ParserResult generate_parser(const OutputContext &ctx,
                             const ProtocolInfo &info) {
  ParserResult result;
  std::ostringstream header;
  std::ostringstream source;
  std::string guard = ctx.header_guard("parser.hpp");

  const auto &read_transitions = info.read_transitions();
  const auto &states = info.states();

  // Header file
  header << "// Auto-generated by NetworkProtocolDSL - do not edit\n";
  header << "#ifndef " << guard << "\n";
  header << "#define " << guard << "\n";
  header << "\n";
  header << "#include <cstddef>\n";
  header << "#include <optional>\n";
  header << "#include <string>\n";
  header << "#include <string_view>\n";
  header << "#include <variant>\n";
  header << "#include <vector>\n";
  header << "#include \"data_types.hpp\"\n";
  header << "#include \"states.hpp\"\n";
  header << "\n";
  header << ctx.open_namespace();
  header << "\n";

  // Parse result enum
  header << "enum class ParseStatus {\n";
  header << "    NeedMoreData,  // Not enough bytes to complete parsing\n";
  header << "    Complete,      // Message parsed successfully\n";
  header << "    Error          // Protocol error\n";
  header << "};\n\n";

  // Parse result struct
  header << "struct ParseResult {\n";
  header << "    ParseStatus status;\n";
  header << "    size_t consumed;  // Number of bytes consumed from input\n";
  header << "};\n\n";

  // Generate individual message parsers for read transitions
  for (const auto &rt : read_transitions) {
    generate_message_parser_header(header, rt);
  }

  // Generate main Parser class
  header << "// Main protocol parser - dispatches to message-specific "
            "parsers\n";
  header << "class Parser {\n";
  header << "public:\n";
  header << "    Parser() = default;\n";
  header << "    \n";
  header << "    // Set the current state for parsing context\n";
  header << "    void set_state(State state) { current_state_ = state; }\n";
  header << "    State get_state() const { return current_state_; }\n";
  header << "    \n";
  header << "    // Parse input bytes\n";
  header << "    ParseResult parse(std::string_view input);\n";
  header << "    \n";
  header << "    // Get current parsing status without consuming more input\n";
  header << "    ParseStatus status() const;\n";
  header << "    \n";
  header << "    // Check if a complete message is available\n";
  header << "    bool has_message() const { return has_message_; }\n";
  header << "    \n";
  header << "    // Get the active parser index (for retrieving parsed data)\n";
  header << "    // Index values:\n";
  for (size_t i = 0; i < read_transitions.size(); ++i) {
    header << "    //   " << i << " = " << read_transitions[i].identifier << "\n";
  }
  header << "    size_t active_parser_index() const { return active_parser_; }\n";
  header << "    \n";
  // Generate take methods for each message type
  for (size_t i = 0; i < read_transitions.size(); ++i) {
    const auto &rt = read_transitions[i];
    header << "    // Take parsed " << rt.identifier << " data (only valid when active_parser_index() == " << i << ")\n";
    header << "    " << rt.identifier << "Data take_" << rt.identifier << "();\n";
  }
  header << "    \n";
  header << "    // Signal end of input stream (for protocols that use EOF as terminator)\n";
  header << "    void on_eof();\n";
  header << "    \n";
  header << "    // Check if EOF has been received\n";
  header << "    bool eof_received() const { return eof_received_; }\n";
  header << "    \n";
  header << "    // Reset the parser\n";
  header << "    void reset();\n";
  header << "\n";
  header << "private:\n";
  if (!states.empty()) {
    header << "    State current_state_ = State::"
           << state_name_to_identifier(*states.begin()) << ";\n";
  } else {
    header << "    State current_state_{};\n";
  }
  header << "    bool has_message_ = false;\n";
  header << "    bool eof_received_ = false;\n";
  header << "    size_t active_parser_ = 0;\n";
  // Add individual parser instances
  for (const auto &rt : read_transitions) {
    header << "    " << rt.identifier << "Parser " << rt.identifier << "_parser_;\n";
  }
  header << "};\n\n";

  header << ctx.close_namespace();
  header << "\n";
  header << "#endif // " << guard << "\n";

  // Source file
  source << "// Auto-generated by NetworkProtocolDSL - do not edit\n";
  source << "#include \"parser.hpp\"\n";
  source << "\n";
  source << "#include <cstring>\n";
  source << "#include <string>\n";
  source << "\n";
  source << ctx.open_namespace();
  source << "\n";

  // Generate implementation for each message parser
  for (const auto &rt : read_transitions) {
    generate_message_parser_reset(source, rt);
    generate_message_parser_parse(source, rt);
    generate_message_parser_take_data(source, rt);
  }

  // Main Parser implementation
  // First, we need to build a map of which parsers are valid for which states
  // Group read transitions by their when_state
  std::map<std::string, std::vector<size_t>> parsers_for_state;
  for (size_t i = 0; i < read_transitions.size(); ++i) {
    parsers_for_state[read_transitions[i].when_state].push_back(i);
  }

  source << "ParseResult Parser::parse(std::string_view input) {\n";
  source << "    ParseResult result;\n";
  source << "    \n";
  source << "    switch (current_state_) {\n";
  
  for (const auto &[state_name, parser_indices] : parsers_for_state) {
    source << "    case State::" << state_name_to_identifier(state_name) << ": {\n";
    
    if (parser_indices.size() == 1) {
      // Only one possible message for this state - dispatch directly
      const auto &rt = read_transitions[parser_indices[0]];
      source << "        result = " << rt.identifier << "_parser_.parse(input);\n";
      source << "        if (result.status == ParseStatus::Complete) {\n";
      source << "            has_message_ = true;\n";
      source << "            active_parser_ = " << parser_indices[0] << ";\n";
      source << "        }\n";
    } else {
      // Multiple possible messages - use lookahead to select parser
      // Collect lookahead prefixes for each parser
      std::vector<std::pair<std::string, size_t>> prefixes;
      for (size_t idx : parser_indices) {
        std::string prefix = get_lookahead_prefix(read_transitions[idx]);
        prefixes.emplace_back(prefix, idx);
      }
      
      // Check if all parsers have unique non-empty prefixes
      bool can_use_lookahead = true;
      std::set<std::string> seen_prefixes;
      for (const auto &[prefix, idx] : prefixes) {
        if (prefix.empty() || seen_prefixes.count(prefix) > 0) {
          can_use_lookahead = false;
          break;
        }
        seen_prefixes.insert(prefix);
      }
      
      if (can_use_lookahead) {
        // Generate lookahead-based dispatch
        source << "        // Use lookahead to select parser based on first bytes\n";
        
        // Find minimum prefix length needed to distinguish
        size_t min_len = 1;
        for (const auto &[prefix, idx] : prefixes) {
          min_len = std::max(min_len, prefix.size());
        }
        
        source << "        if (input.size() < " << min_len << ") {\n";
        source << "            return {ParseStatus::NeedMoreData, 0};\n";
        source << "        }\n";
        
        bool first = true;
        for (const auto &[prefix, idx] : prefixes) {
          const auto &rt = read_transitions[idx];
          std::string escaped = OutputContext::escape_string_literal(prefix);
          
          if (first) {
            source << "        if (";
            first = false;
          } else {
            source << " else if (";
          }
          
          source << "std::memcmp(input.data(), " << escaped << ", " << prefix.size() << ") == 0) {\n";
          source << "            result = " << rt.identifier << "_parser_.parse(input);\n";
          source << "            if (result.status == ParseStatus::Complete) {\n";
          source << "                has_message_ = true;\n";
          source << "                active_parser_ = " << idx << ";\n";
          source << "            }\n";
          source << "        }";
        }
        source << " else {\n";
        source << "            return {ParseStatus::Error, 0};\n";
        source << "        }\n";
      } else {
        // Fall back to sequential try-and-fallback
        source << "        // Try each possible message parser for this state\n";
        for (size_t i = 0; i < parser_indices.size(); ++i) {
          const auto &rt = read_transitions[parser_indices[i]];
          if (i == 0) {
            source << "        result = " << rt.identifier << "_parser_.parse(input);\n";
            source << "        if (result.status == ParseStatus::Complete) {\n";
            source << "            has_message_ = true;\n";
            source << "            active_parser_ = " << parser_indices[i] << ";\n";
            source << "        }";
          } else {
            source << " else if (result.status == ParseStatus::Error) {\n";
            source << "            // Try next parser\n";
            source << "            " << read_transitions[parser_indices[i-1]].identifier << "_parser_.reset();\n";
            source << "            result = " << rt.identifier << "_parser_.parse(input);\n";
            source << "            if (result.status == ParseStatus::Complete) {\n";
            source << "                has_message_ = true;\n";
            source << "                active_parser_ = " << parser_indices[i] << ";\n";
            source << "            }\n";
            source << "        }";
          }
        }
        source << "\n";
      }
    }
    
    source << "        break;\n";
    source << "    }\n";
  }
  
  source << "    default:\n";
  source << "        return {ParseStatus::Error, 0};\n";
  source << "    }\n";
  source << "    \n";
  source << "    return result;\n";
  source << "}\n\n";

  // Generate take methods for each message type
  for (size_t i = 0; i < read_transitions.size(); ++i) {
    const auto &rt = read_transitions[i];
    source << rt.identifier << "Data Parser::take_" << rt.identifier << "() {\n";
    source << "    has_message_ = false;\n";
    source << "    return " << rt.identifier << "_parser_.take_data();\n";
    source << "}\n\n";
  }

  // Generate status() method
  source << "ParseStatus Parser::status() const {\n";
  source << "    if (has_message_) {\n";
  source << "        return ParseStatus::Complete;\n";
  source << "    }\n";
  source << "    return ParseStatus::NeedMoreData;\n";
  source << "}\n\n";

  // Generate on_eof() method
  source << "void Parser::on_eof() {\n";
  source << "    // EOF can be a valid terminator for some protocols\n";
  source << "    // Mark the stream as ended - individual parsers may use this\n";
  source << "    // to complete parsing if they were waiting for more data\n";
  source << "    eof_received_ = true;\n";
  source << "}\n\n";

  source << "void Parser::reset() {\n";
  source << "    has_message_ = false;\n";
  source << "    eof_received_ = false;\n";
  for (const auto &rt : read_transitions) {
    source << "    " << rt.identifier << "_parser_.reset();\n";
  }
  source << "}\n\n";

  source << ctx.close_namespace();

  result.header = header.str();
  result.source = source.str();
  return result;
}

} // namespace networkprotocoldsl::codegen
