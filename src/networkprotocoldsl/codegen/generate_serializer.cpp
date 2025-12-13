#include <networkprotocoldsl/codegen/generate_serializer.hpp>
#include <networkprotocoldsl/codegen/typemapping.hpp>

#include <sstream>

namespace networkprotocoldsl::codegen {

namespace {

// Check if a field is a numeric type that needs std::to_string() conversion
bool is_numeric_type(const std::shared_ptr<const parser::tree::MessageData> &data,
                     const std::string &field_name) {
  if (!data) return false;
  
  auto it = data->find(field_name);
  if (it == data->end()) return false;
  
  const auto &type = it->second;
  if (!type || !type->name) return false;
  
  return type->name->name == "int";
}

// Generate the appropriate write expression for a field
std::string generate_write_expression(
    const std::shared_ptr<const parser::tree::MessageData> &data,
    const std::string &field_name,
    const std::string &access_expr) {
  if (is_numeric_type(data, field_name)) {
    return "std::to_string(" + access_expr + ")";
  }
  return access_expr;
}

// Generate code to apply escape replacement to a string value
// When serializing, replace escape_char with escape_sequence
void generate_escape_replacement_code(std::ostringstream &source,
                                      const std::string &indent,
                                      const std::string &source_expr,
                                      const sema::ast::action::EscapeInfo &escape) {
  std::string escape_char_escaped = OutputContext::escape_string_literal(escape.character);
  std::string escape_seq_escaped = OutputContext::escape_string_literal(escape.sequence);
  
  source << indent << "// Apply escape replacement: " << escape_char_escaped 
         << " -> " << escape_seq_escaped << "\n";
  source << indent << "{\n";
  source << indent << "    std::string temp = " << source_expr << ";\n";
  source << indent << "    std::string result;\n";
  source << indent << "    result.reserve(temp.size() * 2); // Estimate\n";
  source << indent << "    static const char escape_char[] = " << escape_char_escaped << ";\n";
  source << indent << "    static const char escape_seq[] = " << escape_seq_escaped << ";\n";
  source << indent << "    constexpr size_t escape_char_len = " << escape.character.size() << ";\n";
  source << indent << "    constexpr size_t escape_seq_len = " << escape.sequence.size() << ";\n";
  source << indent << "    size_t pos = 0;\n";
  source << indent << "    while (pos < temp.size()) {\n";
  source << indent << "        if (pos + escape_char_len <= temp.size() &&\n";
  source << indent << "            std::memcmp(temp.data() + pos, escape_char, escape_char_len) == 0) {\n";
  source << indent << "            result.append(escape_seq, escape_seq_len);\n";
  source << indent << "            pos += escape_char_len;\n";
  source << indent << "        } else {\n";
  source << indent << "            result += temp[pos];\n";
  source << indent << "            ++pos;\n";
  source << indent << "        }\n";
  source << indent << "    }\n";
  source << indent << "    current_chunk_ = std::move(result);\n";
  source << indent << "}\n";
}

// Generate the header declaration for a single message serializer
void generate_message_serializer_header(std::ostringstream &header,
                                        const WriteTransitionInfo &wt) {
  header << "// Serializer for message: " << wt.message_name << "\n";
  header << "class " << wt.identifier << "Serializer {\n";
  header << "public:\n";
  header << "    " << wt.identifier << "Serializer() = default;\n";
  header << "    \n";
  header << "    // Set the data to serialize\n";
  header << "    void set_data(const " << wt.identifier << "Data& data);\n";
  header << "    void set_data(" << wt.identifier << "Data&& data);\n";
  header << "    \n";
  header << "    // Reset the serializer to initial state\n";
  header << "    void reset();\n";
  header << "    \n";
  header << "    // Check if serialization is complete\n";
  header << "    bool is_complete() const { return complete_; }\n";
  header << "    \n";
  header << "    // Get the next chunk of output bytes\n";
  header << "    // Returns the bytes to write. Empty string means no more data available.\n";
  header << "    std::string_view next_chunk();\n";
  header << "    \n";
  header << "    // Mark that the last chunk was successfully written\n";
  header << "    void advance();\n";
  header << "\n";
  header << "private:\n";
  header << "    " << wt.identifier << "Data data_;\n";
  header << "    bool complete_ = true;\n";
  header << "    bool has_data_ = false;\n";
  header << "    size_t stage_ = 0;\n";
  header << "    size_t loop_index_ = 0;\n";
  header << "    size_t loop_stage_ = 0;\n";
  header << "    std::string current_chunk_;\n";
  header << "};\n\n";
}

// Generate the set_data methods for a message serializer
void generate_message_serializer_set_data(std::ostringstream &source,
                                          const WriteTransitionInfo &wt) {
  source << "void " << wt.identifier << "Serializer::set_data(const "
         << wt.identifier << "Data& data) {\n";
  source << "    data_ = data;\n";
  source << "    has_data_ = true;\n";
  source << "    complete_ = false;\n";
  source << "    stage_ = 0;\n";
  source << "    loop_index_ = 0;\n";
  source << "    loop_stage_ = 0;\n";
  source << "}\n\n";

  source << "void " << wt.identifier << "Serializer::set_data("
         << wt.identifier << "Data&& data) {\n";
  source << "    data_ = std::move(data);\n";
  source << "    has_data_ = true;\n";
  source << "    complete_ = false;\n";
  source << "    stage_ = 0;\n";
  source << "    loop_index_ = 0;\n";
  source << "    loop_stage_ = 0;\n";
  source << "}\n\n";
}

// Generate the reset method for a message serializer
void generate_message_serializer_reset(std::ostringstream &source,
                                       const WriteTransitionInfo &wt) {
  source << "void " << wt.identifier << "Serializer::reset() {\n";
  source << "    data_ = {};\n";
  source << "    has_data_ = false;\n";
  source << "    complete_ = true;\n";
  source << "    stage_ = 0;\n";
  source << "    loop_index_ = 0;\n";
  source << "    loop_stage_ = 0;\n";
  source << "    current_chunk_.clear();\n";
  source << "}\n\n";
}

// Count the number of loop stages for generating the loop state machine
size_t count_loop_stages(const std::shared_ptr<const sema::ast::action::Loop> &loop) {
  size_t count = 0;
  for (const auto &action : loop->actions) {
    ++count;
  }
  return count;
}

// Generate the next_chunk method for a message serializer
void generate_message_serializer_next_chunk(std::ostringstream &source,
                                            const WriteTransitionInfo &wt) {
  source << "std::string_view " << wt.identifier
         << "Serializer::next_chunk() {\n";
  source << "    if (complete_ || !has_data_) {\n";
  source << "        return {};\n";
  source << "    }\n";
  source << "    \n";
  source << "    current_chunk_.clear();\n";
  source << "    \n";
  source << "    switch (stage_) {\n";

  size_t stage = 0;
  for (const auto &action : wt.actions) {
    source << "    case " << stage << ":\n";

    std::visit(
        [&](const auto &a) {
          using T = std::decay_t<decltype(*a)>;

          if constexpr (std::is_same_v<T, sema::ast::action::WriteStaticOctets>) {
            // Write static octets
            std::string escaped =
                OutputContext::escape_string_literal(a->octets);
            source << "        // Write static octets\n";
            source << "        current_chunk_ = " << escaped << ";\n";
            source << "        break;\n";
          } else if constexpr (std::is_same_v<T, sema::ast::action::WriteFromIdentifier>) {
            // Write from identifier - need to convert to string if numeric
            if (a->identifier) {
              std::string field = a->identifier->name;
              // Check if it's a nested field (contains '.')
              size_t dot_pos = field.find('.');
              if (dot_pos != std::string::npos) {
                // Nested field like "header.key" in a loop
                std::string array_name = field.substr(0, dot_pos);
                std::string member_name = field.substr(dot_pos + 1);
                std::string access_expr = "data_." + array_name + "[loop_index_]." + member_name;
                source << "        // Write from nested identifier: " << field << "\n";
                if (a->escape.has_value()) {
                  generate_escape_replacement_code(source, "        ", access_expr, a->escape.value());
                } else {
                  source << "        current_chunk_ = " << access_expr << ";\n";
                }
              } else {
                std::string access_expr = "data_." + field;
                std::string write_expr = generate_write_expression(wt.data, field, access_expr);
                source << "        // Write from identifier: " << field << "\n";
                if (a->escape.has_value()) {
                  generate_escape_replacement_code(source, "        ", write_expr, a->escape.value());
                } else {
                  source << "        current_chunk_ = " << write_expr << ";\n";
                }
              }
              source << "        break;\n";
            }
          } else if constexpr (std::is_same_v<T, sema::ast::action::Loop>) {
            // Handle loop
            std::string array_field = a->collection->name;
            std::string loop_var = a->variable->name;
            source << "        // Loop over: " << array_field << "\n";
            source << "        if (loop_index_ >= data_." << array_field << ".size()) {\n";
            source << "            // Loop complete, advance to next stage\n";
            source << "            loop_index_ = 0;\n";
            source << "            loop_stage_ = 0;\n";
            source << "            ++stage_;\n";
            source << "            return next_chunk();  // Recurse to next stage\n";
            source << "        }\n";
            source << "        \n";
            source << "        switch (loop_stage_) {\n";

            size_t loop_stage = 0;
            for (const auto &loop_action : a->actions) {
              source << "        case " << loop_stage << ":\n";

              std::visit(
                  [&](const auto &la) {
                    using LT = std::decay_t<decltype(*la)>;

                    if constexpr (std::is_same_v<LT, sema::ast::action::WriteStaticOctets>) {
                      std::string escaped =
                          OutputContext::escape_string_literal(la->octets);
                      source << "            current_chunk_ = " << escaped << ";\n";
                      source << "            break;\n";
                    } else if constexpr (std::is_same_v<LT, sema::ast::action::WriteFromIdentifier>) {
                      if (la->identifier) {
                        std::string field = la->identifier->name;
                        size_t dot_pos = field.find('.');
                        std::string access_expr;
                        if (dot_pos != std::string::npos) {
                          std::string member_name = field.substr(dot_pos + 1);
                          access_expr = "data_." + array_field + "[loop_index_]." + member_name;
                        } else {
                          access_expr = "data_." + field;
                        }
                        if (la->escape.has_value()) {
                          generate_escape_replacement_code(source, "            ", access_expr, la->escape.value());
                        } else {
                          source << "            current_chunk_ = " << access_expr << ";\n";
                        }
                        source << "            break;\n";
                      }
                    }
                  },
                  loop_action);

              ++loop_stage;
            }

            source << "        default:\n";
            source << "            // All loop stages done for this element\n";
            source << "            ++loop_index_;\n";
            source << "            loop_stage_ = 0;\n";
            source << "            return next_chunk();  // Recurse\n";
            source << "        }\n";
            source << "        break;\n";
          }
        },
        action);

    ++stage;
  }

  // Final stage - mark complete
  source << "    default:\n";
  source << "        complete_ = true;\n";
  source << "        return {};\n";
  source << "    }\n";
  source << "    \n";
  source << "    return current_chunk_;\n";
  source << "}\n\n";
}

// Generate the advance method for a message serializer
void generate_message_serializer_advance(std::ostringstream &source,
                                         const WriteTransitionInfo &wt) {
  source << "void " << wt.identifier << "Serializer::advance() {\n";
  source << "    if (complete_) return;\n";
  source << "    \n";

  size_t total_stages = wt.actions.size();

  // Need to check if current stage is a loop
  source << "    switch (stage_) {\n";

  size_t stage = 0;
  for (const auto &action : wt.actions) {
    bool is_loop = std::visit(
        [](const auto &a) {
          using T = std::decay_t<decltype(*a)>;
          return std::is_same_v<T, sema::ast::action::Loop>;
        },
        action);

    if (is_loop) {
      auto loop = std::get<std::shared_ptr<const sema::ast::action::Loop>>(action);
      size_t loop_stages = count_loop_stages(loop);
      source << "    case " << stage << ":\n";
      source << "        // Loop stage - advance within loop\n";
      source << "        ++loop_stage_;\n";
      source << "        if (loop_stage_ >= " << loop_stages << ") {\n";
      source << "            ++loop_index_;\n";
      source << "            loop_stage_ = 0;\n";
      source << "        }\n";
      source << "        break;\n";
    }
    ++stage;
  }

  source << "    default:\n";
  source << "        ++stage_;\n";
  source << "        if (stage_ >= " << total_stages << ") {\n";
  source << "            complete_ = true;\n";
  source << "        }\n";
  source << "        break;\n";
  source << "    }\n";
  source << "}\n\n";
}

} // anonymous namespace

SerializerResult generate_serializer(const OutputContext &ctx,
                                     const ProtocolInfo &info) {
  SerializerResult result;
  std::ostringstream header;
  std::ostringstream source;
  std::string guard = ctx.header_guard("serializer.hpp");

  const auto &write_transitions = info.write_transitions();

  // Header file
  header << "// This file is auto-generated. Do not edit.\n\n";
  header << "#ifndef " << guard << "\n";
  header << "#define " << guard << "\n";
  header << "\n";
  header << "#include <cstddef>\n";
  header << "#include <string>\n";
  header << "#include <string_view>\n";
  header << "#include <utility>\n";
  header << "#include \"data_types.hpp\"\n";
  header << "#include \"states.hpp\"\n";
  header << "\n";
  header << ctx.open_namespace();
  header << "\n";

  // Generate individual message serializers for write transitions
  for (const auto &wt : write_transitions) {
    generate_message_serializer_header(header, wt);
  }

  // Generate main Serializer class (can hold any message serializer)
  header << "// Main protocol serializer - provides unified interface\n";
  header << "class Serializer {\n";
  header << "public:\n";
  header << "    Serializer() = default;\n";
  header << "    \n";
  header << "    // Reset the serializer\n";
  header << "    void reset();\n";
  header << "    \n";
  header << "    // Check if serialization is complete\n";
  header << "    bool is_complete() const { return is_complete_; }\n";
  header << "\n";
  header << "private:\n";
  header << "    bool is_complete_ = true;\n";
  header << "};\n\n";

  header << ctx.close_namespace();
  header << "\n";
  header << "#endif // " << guard << "\n";

  // Source file
  source << "// This file is auto-generated. Do not edit.\n\n";
  source << "#include \"serializer.hpp\"\n";
  source << "#include <cstring>\n";  // For std::memcmp in escape replacement
  source << "\n";
  source << ctx.open_namespace();
  source << "\n";

  // Generate implementation for each message serializer
  for (const auto &wt : write_transitions) {
    generate_message_serializer_set_data(source, wt);
    generate_message_serializer_reset(source, wt);
    generate_message_serializer_next_chunk(source, wt);
    generate_message_serializer_advance(source, wt);
  }

  // Main Serializer implementation
  source << "void Serializer::reset() {\n";
  source << "    is_complete_ = true;\n";
  source << "}\n\n";

  source << ctx.close_namespace();

  result.header = header.str();
  result.source = source.str();
  return result;
}

} // namespace networkprotocoldsl::codegen
