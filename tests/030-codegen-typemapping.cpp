#include <networkprotocoldsl/codegen/typemapping.hpp>
#include <networkprotocoldsl/parser/tree/type.hpp>

#include <gtest/gtest.h>

using namespace networkprotocoldsl::codegen;
using namespace networkprotocoldsl::parser::tree;

// Helper to create a Type with parameters
std::shared_ptr<const Type> make_type(const std::string &name,
                                      const TypeParameterMap &params = {}) {
  auto type = std::make_shared<Type>();
  auto ident = std::make_shared<IdentifierReference>();
  ident->name = name;
  type->name = ident;
  type->parameters = std::make_shared<TypeParameterMap>(params);
  return type;
}

// Helper to create TypeParameterValue from int
TypeParameterValue make_int_param(int value) {
  auto literal = std::make_shared<IntegerLiteral>();
  literal->value = value;
  return std::make_shared<const IntegerLiteral>(*literal);
}

// Helper to create TypeParameterValue from bool
TypeParameterValue make_bool_param(bool value) {
  auto literal = std::make_shared<BooleanLiteral>();
  literal->value = value;
  return std::make_shared<const BooleanLiteral>(*literal);
}

TEST(TypeMappingTest, IntUnsigned8Bits) {
  TypeParameterMap params;
  params["unsigned"] = make_bool_param(true);
  params["bits"] = make_int_param(8);

  auto type = make_type("int", params);
  auto result = type_to_cpp(type, "test_field");

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->cpp_type, "uint8_t");
  EXPECT_TRUE(result->auxiliary_definitions.empty());
}

TEST(TypeMappingTest, IntSigned32Bits) {
  TypeParameterMap params;
  params["unsigned"] = make_bool_param(false);
  params["bits"] = make_int_param(32);

  auto type = make_type("int", params);
  auto result = type_to_cpp(type, "test_field");

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->cpp_type, "int32_t");
}

TEST(TypeMappingTest, IntUnsigned64Bits) {
  TypeParameterMap params;
  params["unsigned"] = make_bool_param(true);
  params["bits"] = make_int_param(64);

  auto type = make_type("int", params);
  auto result = type_to_cpp(type, "test_field");

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->cpp_type, "uint64_t");
}

TEST(TypeMappingTest, IntDefaultsTo32Bits) {
  TypeParameterMap params;
  // No bits parameter - should default to 32

  auto type = make_type("int", params);
  auto result = type_to_cpp(type, "test_field");

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->cpp_type, "int32_t");
}

TEST(TypeMappingTest, IntUnsignedDefaultsToFalse) {
  TypeParameterMap params;
  params["bits"] = make_int_param(16);
  // No unsigned parameter - should default to false

  auto type = make_type("int", params);
  auto result = type_to_cpp(type, "test_field");

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->cpp_type, "int16_t");
}

TEST(TypeMappingTest, StrType) {
  auto type = make_type("str", {});
  auto result = type_to_cpp(type, "test_field");

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->cpp_type, "std::string");
}

TEST(TypeMappingTest, ArrayOfStrings) {
  // Create element type (str)
  auto str_type = make_type("str", {});

  // Create array type
  TypeParameterMap array_params;
  array_params["element_type"] = str_type;

  auto type = make_type("array", array_params);
  auto result = type_to_cpp(type, "test_field");

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->cpp_type, "std::vector<std::string>");
}

TEST(TypeMappingTest, ArrayOfInts) {
  // Create element type (int)
  TypeParameterMap int_params;
  int_params["bits"] = make_int_param(32);
  auto int_type = make_type("int", int_params);

  // Create array type
  TypeParameterMap array_params;
  array_params["element_type"] = int_type;

  auto type = make_type("array", array_params);
  auto result = type_to_cpp(type, "test_field");

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->cpp_type, "std::vector<int32_t>");
}

TEST(TypeMappingTest, MessageNameToIdentifier) {
  EXPECT_EQ(message_name_to_identifier("SMTP Server Greeting"),
            "SMTPServerGreeting");
  EXPECT_EQ(message_name_to_identifier("HTTP Request"), "HTTPRequest");
  EXPECT_EQ(message_name_to_identifier("simple"), "Simple");
  EXPECT_EQ(message_name_to_identifier("Client Closes Connection"),
            "ClientClosesConnection");
}

TEST(TypeMappingTest, StateNameToIdentifier) {
  EXPECT_EQ(state_name_to_identifier("Open"), "Open");
  EXPECT_EQ(state_name_to_identifier("Closed"), "Closed");
  EXPECT_EQ(state_name_to_identifier("AwaitResponse"), "AwaitResponse");
}

TEST(TypeMappingTest, UnknownTypeReturnsNullopt) {
  auto type = make_type("unknown_type", {});
  auto result = type_to_cpp(type, "test_field");

  EXPECT_FALSE(result.has_value());
}

TEST(TypeMappingTest, NullTypeReturnsNullopt) {
  auto result = type_to_cpp(nullptr, "test_field");

  EXPECT_FALSE(result.has_value());
}

TEST(TypeMappingTest, ArrayMissingElementTypeReturnsNullopt) {
  // Array with no element_type parameter
  auto type = make_type("array", {});
  auto result = type_to_cpp(type, "test_field");

  EXPECT_FALSE(result.has_value());
}

TEST(TypeMappingTest, TupleType) {
  // Create a tuple with two fields
  TypeParameterMap tuple_params;
  
  // First field: key: str
  tuple_params["key"] = make_type("str", {});
  
  // Second field: value: int
  TypeParameterMap int_params;
  int_params["bits"] = make_int_param(32);
  tuple_params["value"] = make_type("int", int_params);
  
  auto type = make_type("tuple", tuple_params);
  auto result = type_to_cpp(type, "my_tuple");
  
  ASSERT_TRUE(result.has_value());
  // Tuple should generate a struct name matching the prefix
  EXPECT_EQ(result->cpp_type, "my_tuple");
  // Should have auxiliary definitions for the struct
  EXPECT_FALSE(result->auxiliary_definitions.empty());
  EXPECT_TRUE(result->auxiliary_definitions.find("struct my_tuple") != std::string::npos);
  EXPECT_TRUE(result->auxiliary_definitions.find("std::string key") != std::string::npos);
  EXPECT_TRUE(result->auxiliary_definitions.find("int32_t value") != std::string::npos);
}

TEST(TypeMappingTest, ArrayOfTuples) {
  // Create a tuple type
  TypeParameterMap tuple_params;
  tuple_params["name"] = make_type("str", {});
  auto tuple_type = make_type("tuple", tuple_params);
  
  // Create array of tuple
  TypeParameterMap array_params;
  array_params["element_type"] = tuple_type;
  
  auto type = make_type("array", array_params);
  auto result = type_to_cpp(type, "items");
  
  ASSERT_TRUE(result.has_value());
  // Array of tuples uses prefix + "Element" for element type
  EXPECT_EQ(result->cpp_type, "std::vector<itemsElement>");
  EXPECT_FALSE(result->auxiliary_definitions.empty());
}

TEST(TypeMappingTest, IntInvalidBits) {
  // Test with non-standard bit size (e.g., 24 bits)
  TypeParameterMap params;
  params["bits"] = make_int_param(24);
  
  auto type = make_type("int", params);
  auto result = type_to_cpp(type, "test_field");
  
  // Should still return a result, likely defaulting to 32 or returning nullopt
  // depending on implementation
  // For now, just verify it doesn't crash
  (void)result;
}

TEST(TypeMappingTest, MessageNameWithNumbers) {
  EXPECT_EQ(message_name_to_identifier("HTTP 1.1 Request"),
            "HTTP11Request");
}

TEST(TypeMappingTest, MessageNameWithSpecialChars) {
  EXPECT_EQ(message_name_to_identifier("SMTP:EHLO Command"),
            "SMTPEHLOCommand");
}

TEST(TypeMappingTest, MessageNameAllLowercase) {
  EXPECT_EQ(message_name_to_identifier("simple message"),
            "SimpleMessage");
}

TEST(TypeMappingTest, StateNameWithSpaces) {
  // State names shouldn't have spaces in well-formed protocols,
  // but test the behavior anyway - spaces are removed
  EXPECT_EQ(state_name_to_identifier("Await Response"), "AwaitResponse");
}

TEST(TypeMappingTest, StateNameStartingWithDigit) {
  // State names starting with digits should be prefixed with underscore
  EXPECT_EQ(state_name_to_identifier("123State"), "_123State");
}

TEST(TypeMappingTest, NestedTupleType) {
  // Create inner tuple: tuple<a=int, b=str>
  TypeParameterMap inner_tuple_params;
  TypeParameterMap int_params;
  int_params["bits"] = make_int_param(32);
  inner_tuple_params["a"] = make_type("int", int_params);
  inner_tuple_params["b"] = make_type("str", {});
  auto inner_tuple = make_type("tuple", inner_tuple_params);

  // Create outer tuple: tuple<nested=tuple<a=int, b=str>, name=str>
  TypeParameterMap outer_tuple_params;
  outer_tuple_params["nested"] = inner_tuple;
  outer_tuple_params["name"] = make_type("str", {});

  auto type = make_type("tuple", outer_tuple_params);
  auto result = type_to_cpp(type, "OuterTuple");

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->cpp_type, "OuterTuple");
  EXPECT_TRUE(result->is_struct);
  
  // Should contain definition for outer struct
  EXPECT_TRUE(result->auxiliary_definitions.find("struct OuterTuple") != std::string::npos);
  // Should contain definition for nested struct with proper naming
  EXPECT_TRUE(result->auxiliary_definitions.find("struct OuterTuple_nested") != std::string::npos);
  // Outer struct should reference nested struct type
  EXPECT_TRUE(result->auxiliary_definitions.find("OuterTuple_nested nested") != std::string::npos);
  // Nested struct should have its fields
  EXPECT_TRUE(result->auxiliary_definitions.find("int32_t a") != std::string::npos);
  EXPECT_TRUE(result->auxiliary_definitions.find("std::string b") != std::string::npos);
}

TEST(TypeMappingTest, ArrayOfNestedTuples) {
  // Create inner tuple: tuple<id=int>
  TypeParameterMap inner_tuple_params;
  TypeParameterMap int_params;
  int_params["bits"] = make_int_param(16);
  inner_tuple_params["id"] = make_type("int", int_params);
  auto inner_tuple = make_type("tuple", inner_tuple_params);

  // Create outer tuple: tuple<items=array<element_type=tuple<id=int>>>
  TypeParameterMap array_params;
  array_params["element_type"] = inner_tuple;
  auto array_type = make_type("array", array_params);

  TypeParameterMap outer_tuple_params;
  outer_tuple_params["items"] = array_type;

  auto type = make_type("tuple", outer_tuple_params);
  auto result = type_to_cpp(type, "Container");

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->cpp_type, "Container");
  // Should contain vector of element struct
  EXPECT_TRUE(result->auxiliary_definitions.find("std::vector<") != std::string::npos);
}

TEST(TypeMappingTest, EmptyTupleReturnsNullopt) {
  // Empty tuple should return nullopt
  auto type = make_type("tuple", {});
  auto result = type_to_cpp(type, "empty_tuple");

  EXPECT_FALSE(result.has_value());
}
