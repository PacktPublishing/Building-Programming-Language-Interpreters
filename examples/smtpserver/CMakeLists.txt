# Add a custom command to generate the embedded file.

set(SMTP_SOURCE_BASENAME smtp.networkprotocoldsl)
set(SMTP_SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${SMTP_SOURCE_BASENAME})
set(SMTP_STRING_LITERAL_FILE ${CMAKE_CURRENT_BINARY_DIR}/${SMTP_SOURCE_BASENAME}.literal)
add_custom_command(
    OUTPUT ${SMTP_STRING_LITERAL_FILE}
    COMMAND ${CMAKE_COMMAND} 
            -DINPUT_FILE="${SMTP_SOURCE_FILE}"
            -DOUTPUT_FILE="${SMTP_STRING_LITERAL_FILE}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/generate_string_literals.cmake"
    DEPENDS ${SMTP_SOURCE_FILE} generate_string_literals.cmake
    COMMENT "Generating ${SMTP_STRING_LITERAL_FILE} from ${SMTP_SOURCE_FILE}"
)

# Instead of defining a separate target, mark main.cpp as dependent on the generated file.
set_source_files_properties(
    interpreted_program.cpp PROPERTIES OBJECT_DEPENDS ${SMTP_STRING_LITERAL_FILE}
)

# Build the smtpserver executable and make it depend on the generated file.
find_package(CLI11 REQUIRED)

add_executable(smtpserver 
    main.cpp 
    interpreted_program.cpp 
    interpreted_program.hpp
    server_core.cpp
    server_core.hpp
    server_processor.cpp
    server_processor.hpp
)
target_link_libraries(smtpserver PRIVATE networkprotocoldsl networkprotocoldsl_uv CLI11::CLI11)
target_compile_definitions(smtpserver PRIVATE -DSMTP_NETWORKPROTOCOLDSL_LITERAL="${SMTP_STRING_LITERAL_FILE}")
