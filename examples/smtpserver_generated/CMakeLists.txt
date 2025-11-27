# CMakeLists.txt for generated SMTP server example
#
# This example uses the protocol_generator to generate C++ code from the
# SMTP protocol definition, then builds an SMTP server using the generated
# code with type-safe callbacks.

# Path to the protocol definition file
set(SMTP_SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../smtpserver/smtp.networkprotocoldsl)

# Output directory for generated files
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

# List of generated source files
set(GENERATED_SOURCES
    ${GENERATED_DIR}/data_types.cpp
    ${GENERATED_DIR}/states.cpp
    ${GENERATED_DIR}/parser.cpp
    ${GENERATED_DIR}/serializer.cpp
    ${GENERATED_DIR}/state_machine.cpp
    ${GENERATED_DIR}/runner.cpp
)

set(GENERATED_HEADERS
    ${GENERATED_DIR}/data_types.hpp
    ${GENERATED_DIR}/states.hpp
    ${GENERATED_DIR}/parser.hpp
    ${GENERATED_DIR}/serializer.hpp
    ${GENERATED_DIR}/state_machine.hpp
    ${GENERATED_DIR}/runner.hpp
    ${GENERATED_DIR}/protocol.hpp
)

# Create directory for generated files
file(MAKE_DIRECTORY ${GENERATED_DIR})

# Custom command to generate the protocol code
add_custom_command(
    OUTPUT ${GENERATED_SOURCES} ${GENERATED_HEADERS}
    COMMAND protocol_generator 
            ${SMTP_SOURCE_FILE}
            --output ${GENERATED_DIR}
            --namespace "smtp::generated"
            --library "smtp_protocol"
    DEPENDS ${SMTP_SOURCE_FILE} protocol_generator
    COMMENT "Generating SMTP protocol code from ${SMTP_SOURCE_FILE}"
)

# Create a library from the generated code
add_library(smtp_protocol_generated STATIC ${GENERATED_SOURCES})
target_include_directories(smtp_protocol_generated PUBLIC ${GENERATED_DIR})
target_compile_features(smtp_protocol_generated PUBLIC cxx_std_20)

# Find CLI11 for command-line parsing
find_package(CLI11 REQUIRED)

# Build the smtpserver_generated executable
add_executable(smtpserver_generated
    main.cpp
    smtp_handler.cpp
    smtp_handler.hpp
)

target_link_libraries(smtpserver_generated PRIVATE 
    smtp_protocol_generated 
    networkprotocoldsl_uv 
    CLI11::CLI11
)
target_include_directories(smtpserver_generated PRIVATE ${GENERATED_DIR})
target_compile_features(smtpserver_generated PRIVATE cxx_std_20)
