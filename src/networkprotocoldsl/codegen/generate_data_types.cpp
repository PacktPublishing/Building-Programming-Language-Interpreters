#include <networkprotocoldsl/codegen/generate_data_types.hpp>
#include <networkprotocoldsl/codegen/typemapping.hpp>

#include <sstream>

namespace networkprotocoldsl::codegen {

DataTypesResult generate_data_types(const OutputContext &ctx,
                                    const ProtocolInfo &info) {
  DataTypesResult result;
  std::ostringstream header;
  std::ostringstream source;
  std::string guard = ctx.header_guard("data_types.hpp");

  // Header file
  header << "#ifndef " << guard << "\n";
  header << "#define " << guard << "\n";
  header << "\n";
  header << "#include <cstdint>\n";
  header << "#include <memory>\n";
  header << "#include <string>\n";
  header << "#include <vector>\n";
  header << "\n";
  header << ctx.open_namespace();
  header << "\n";

  // Generate data struct for each message
  for (const auto &msg : info.messages()) {
    if (!msg.data || msg.data->empty()) {
      // Generate an empty struct for messages with no data
      header << "struct " << msg.identifier << "Data {\n";
      header << "};\n\n";
      continue;
    }

    // Collect field definitions
    std::ostringstream fields;
    std::ostringstream msg_auxiliary;

    for (const auto &[field_name, field_type] : *msg.data) {
      auto mapping = type_to_cpp(field_type, msg.identifier + "_" + field_name);
      if (!mapping) {
        result.errors.push_back("Failed to map type for field '" + field_name +
                                "' in message '" + msg.name + "'");
        continue;
      }

      if (!mapping->auxiliary_definitions.empty()) {
        msg_auxiliary << mapping->auxiliary_definitions << "\n";
      }

      fields << "    " << mapping->cpp_type << " " << field_name << ";\n";
    }

    // Output auxiliary definitions first (nested structs)
    if (msg_auxiliary.str().length() > 0) {
      header << "// Auxiliary types for " << msg.identifier << "Data\n";
      header << msg_auxiliary.str();
    }

    // Output the main struct
    header << "struct " << msg.identifier << "Data {\n";
    header << fields.str();
    header << "};\n\n";
  }

  header << ctx.close_namespace();
  header << "\n";
  header << "#endif // " << guard << "\n";

  // Source file (mostly empty for data types, but included for consistency)
  source << "#include \"data_types.hpp\"\n";
  source << "\n";
  source << ctx.open_namespace();
  source << "\n";
  source << "// Data type implementations (if needed)\n";
  source << "\n";
  source << ctx.close_namespace();

  result.header = header.str();
  result.source = source.str();
  return result;
}

} // namespace networkprotocoldsl::codegen
