# =============================================================================
# Code Generation Integration Tests
# =============================================================================
# These tests verify the generated code works correctly. The code is generated
# at build time using the protocol_generator, ensuring it's built with the same
# compiler flags (including sanitizers) as the rest of the project.

# Path to the SMTP protocol definition file
set(SMTP_TEST_SOURCE_FILE ${CMAKE_SOURCE_DIR}/examples/smtpserver/smtp.networkprotocoldsl)

# Output directory for generated files for tests
set(CODEGEN_TEST_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated_smtp)

# List of generated source files
set(CODEGEN_TEST_GENERATED_SOURCES
    ${CODEGEN_TEST_GENERATED_DIR}/data_types.cpp
    ${CODEGEN_TEST_GENERATED_DIR}/states.cpp
    ${CODEGEN_TEST_GENERATED_DIR}/parser.cpp
    ${CODEGEN_TEST_GENERATED_DIR}/serializer.cpp
    ${CODEGEN_TEST_GENERATED_DIR}/state_machine.cpp
    ${CODEGEN_TEST_GENERATED_DIR}/runner.cpp
)

set(CODEGEN_TEST_GENERATED_HEADERS
    ${CODEGEN_TEST_GENERATED_DIR}/data_types.hpp
    ${CODEGEN_TEST_GENERATED_DIR}/states.hpp
    ${CODEGEN_TEST_GENERATED_DIR}/parser.hpp
    ${CODEGEN_TEST_GENERATED_DIR}/serializer.hpp
    ${CODEGEN_TEST_GENERATED_DIR}/state_machine.hpp
    ${CODEGEN_TEST_GENERATED_DIR}/runner.hpp
    ${CODEGEN_TEST_GENERATED_DIR}/protocol.hpp
)

# Create directory for generated files
file(MAKE_DIRECTORY ${CODEGEN_TEST_GENERATED_DIR})

# Custom command to generate the protocol code
add_custom_command(
    OUTPUT ${CODEGEN_TEST_GENERATED_SOURCES} ${CODEGEN_TEST_GENERATED_HEADERS}
    COMMAND protocol_generator 
            ${SMTP_TEST_SOURCE_FILE}
            --output ${CODEGEN_TEST_GENERATED_DIR}
            --namespace "smtp::generated"
            --library "smtp_test_protocol"
    DEPENDS ${SMTP_TEST_SOURCE_FILE} protocol_generator
    COMMENT "Generating SMTP protocol code for integration tests"
)

# Create a library from the generated code for tests
add_library(smtp_test_protocol STATIC ${CODEGEN_TEST_GENERATED_SOURCES})
target_include_directories(smtp_test_protocol PUBLIC ${CODEGEN_TEST_GENERATED_DIR})
target_compile_features(smtp_test_protocol PUBLIC cxx_std_20)

# Integration test executables - simple tests without libuv
foreach(
    CODEGEN_TEST
    test_greeting
    test_ehlo_parse
    test_roundtrip
    test_conversation
    test_partial
)
    add_executable(codegen_${CODEGEN_TEST} ${CODEGEN_TEST}.cpp)
    target_link_libraries(codegen_${CODEGEN_TEST} PRIVATE smtp_test_protocol)
    target_include_directories(codegen_${CODEGEN_TEST} PRIVATE ${CODEGEN_TEST_GENERATED_DIR})
    add_test(NAME CodegenIntegration.${CODEGEN_TEST} COMMAND codegen_${CODEGEN_TEST})
endforeach()

# Integration test executables - tests requiring libuv
foreach(
    CODEGEN_TEST
    test_uv_adapter
    test_full_email_transaction
    test_multi_connection
)
    add_executable(codegen_${CODEGEN_TEST} ${CODEGEN_TEST}.cpp)
    target_link_libraries(codegen_${CODEGEN_TEST} PRIVATE 
        smtp_test_protocol 
        uv
        pthread
    )
    target_include_directories(codegen_${CODEGEN_TEST} PRIVATE ${CODEGEN_TEST_GENERATED_DIR})
    add_test(NAME CodegenIntegration.${CODEGEN_TEST} COMMAND codegen_${CODEGEN_TEST})
endforeach()

# Integration test executables - tests requiring libuv and networkprotocoldsl_uv
foreach(
    CODEGEN_TEST
    test_protocol_mismatch
)
    add_executable(codegen_${CODEGEN_TEST} ${CODEGEN_TEST}.cpp)
    target_link_libraries(codegen_${CODEGEN_TEST} PRIVATE 
        smtp_test_protocol 
        networkprotocoldsl_uv
        uv
        pthread
    )
    target_include_directories(codegen_${CODEGEN_TEST} PRIVATE 
        ${CODEGEN_TEST_GENERATED_DIR}
        ${CMAKE_SOURCE_DIR}/src
    )
    add_test(NAME CodegenIntegration.${CODEGEN_TEST} COMMAND codegen_${CODEGEN_TEST})
endforeach()
