#include <networkprotocoldsl/codegen/typemapping.hpp>

#include <algorithm>
#include <cctype>
#include <sstream>

namespace networkprotocoldsl::codegen {

namespace {

// Helper to extract a boolean parameter from type parameters
std::optional<bool>
get_bool_param(const std::shared_ptr<const parser::tree::TypeParameterMap> &params,
               const std::string &name) {
  if (!params) return std::nullopt;
  auto it = params->find(name);
  if (it == params->end()) return std::nullopt;
  
  if (std::holds_alternative<std::shared_ptr<const parser::tree::BooleanLiteral>>(it->second)) {
    return std::get<std::shared_ptr<const parser::tree::BooleanLiteral>>(it->second)->value;
  }
  return std::nullopt;
}

// Helper to extract an integer parameter from type parameters
std::optional<int>
get_int_param(const std::shared_ptr<const parser::tree::TypeParameterMap> &params,
              const std::string &name) {
  if (!params) return std::nullopt;
  auto it = params->find(name);
  if (it == params->end()) return std::nullopt;
  
  if (std::holds_alternative<std::shared_ptr<const parser::tree::IntegerLiteral>>(it->second)) {
    return std::get<std::shared_ptr<const parser::tree::IntegerLiteral>>(it->second)->value;
  }
  return std::nullopt;
}

// Helper to extract a type parameter from type parameters
std::optional<std::shared_ptr<const parser::tree::Type>>
get_type_param(const std::shared_ptr<const parser::tree::TypeParameterMap> &params,
               const std::string &name) {
  if (!params) return std::nullopt;
  auto it = params->find(name);
  if (it == params->end()) return std::nullopt;
  
  if (std::holds_alternative<std::shared_ptr<const parser::tree::Type>>(it->second)) {
    return std::get<std::shared_ptr<const parser::tree::Type>>(it->second);
  }
  return std::nullopt;
}

// Map int type based on parameters
std::optional<TypeMappingResult>
map_int_type(const std::shared_ptr<const parser::tree::Type> &type) {
  auto params = type->parameters;
  
  bool is_unsigned = get_bool_param(params, "unsigned").value_or(false);
  int bits = get_int_param(params, "bits").value_or(32);
  
  std::string cpp_type;
  if (is_unsigned) {
    switch (bits) {
      case 8:  cpp_type = "uint8_t"; break;
      case 16: cpp_type = "uint16_t"; break;
      case 32: cpp_type = "uint32_t"; break;
      case 64: cpp_type = "uint64_t"; break;
      default: cpp_type = "uint32_t"; break;
    }
  } else {
    switch (bits) {
      case 8:  cpp_type = "int8_t"; break;
      case 16: cpp_type = "int16_t"; break;
      case 32: cpp_type = "int32_t"; break;
      case 64: cpp_type = "int64_t"; break;
      default: cpp_type = "int32_t"; break;
    }
  }
  
  return TypeMappingResult{cpp_type, "", false};
}

// Map str type
std::optional<TypeMappingResult>
map_str_type(const std::shared_ptr<const parser::tree::Type> &type) {
  // For now, all string types map to std::string
  // In the future, we could use std::array<char, N> for fixed-size strings
  return TypeMappingResult{"std::string", "", false};
}

// Map array type
std::optional<TypeMappingResult>
map_array_type(const std::shared_ptr<const parser::tree::Type> &type,
               const std::string &struct_name_prefix) {
  auto element_type = get_type_param(type->parameters, "element_type");
  if (!element_type) {
    return std::nullopt;
  }
  
  auto element_mapping = type_to_cpp(*element_type, struct_name_prefix + "Element");
  if (!element_mapping) {
    return std::nullopt;
  }
  
  std::string cpp_type = "std::vector<" + element_mapping->cpp_type + ">";
  return TypeMappingResult{cpp_type, element_mapping->auxiliary_definitions, false};
}

// Map tuple type to a struct
std::optional<TypeMappingResult>
map_tuple_type(const std::shared_ptr<const parser::tree::Type> &type,
               const std::string &struct_name_prefix) {
  if (!type->parameters || type->parameters->empty()) {
    return std::nullopt;
  }
  
  std::string struct_name = struct_name_prefix.empty() ? "TupleData" : struct_name_prefix;
  
  std::ostringstream struct_def;
  std::ostringstream nested_defs;
  
  struct_def << "struct " << struct_name << " {\n";
  
  for (const auto &[field_name, field_value] : *type->parameters) {
    if (!std::holds_alternative<std::shared_ptr<const parser::tree::Type>>(field_value)) {
      continue; // Skip non-type parameters
    }
    
    auto field_type = std::get<std::shared_ptr<const parser::tree::Type>>(field_value);
    auto field_mapping = type_to_cpp(field_type, struct_name + "_" + field_name);
    if (!field_mapping) {
      return std::nullopt;
    }
    
    if (!field_mapping->auxiliary_definitions.empty()) {
      nested_defs << field_mapping->auxiliary_definitions << "\n";
    }
    
    struct_def << "    " << field_mapping->cpp_type << " " << field_name << ";\n";
  }
  
  struct_def << "};\n";
  
  std::string all_defs = nested_defs.str() + struct_def.str();
  return TypeMappingResult{struct_name, all_defs, true};
}

} // anonymous namespace

std::optional<TypeMappingResult>
type_to_cpp(const std::shared_ptr<const parser::tree::Type> &type,
            const std::string &struct_name_prefix) {
  if (!type || !type->name) {
    return std::nullopt;
  }
  
  const std::string &type_name = type->name->name;
  
  if (type_name == "int") {
    return map_int_type(type);
  } else if (type_name == "str") {
    return map_str_type(type);
  } else if (type_name == "array") {
    return map_array_type(type, struct_name_prefix);
  } else if (type_name == "tuple") {
    return map_tuple_type(type, struct_name_prefix);
  }
  
  // Unknown type
  return std::nullopt;
}

std::string message_name_to_identifier(const std::string &message_name) {
  std::string result;
  result.reserve(message_name.size());
  
  bool capitalize_next = true;
  for (char c : message_name) {
    if (c == ' ' || c == '-' || c == '_') {
      capitalize_next = true;
    } else if (std::isalnum(static_cast<unsigned char>(c))) {
      if (capitalize_next) {
        result += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        capitalize_next = false;
      } else {
        result += c;
      }
    }
  }
  
  return result;
}

std::string state_name_to_identifier(const std::string &state_name) {
  // State names are already valid identifiers in the DSL
  // Just ensure they're valid C++ identifiers
  std::string result;
  result.reserve(state_name.size());
  
  for (size_t i = 0; i < state_name.size(); ++i) {
    char c = state_name[i];
    if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
      result += c;
    }
  }
  
  // Ensure it doesn't start with a digit
  if (!result.empty() && std::isdigit(static_cast<unsigned char>(result[0]))) {
    result = "_" + result;
  }
  
  return result;
}

} // namespace networkprotocoldsl::codegen
