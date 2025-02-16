if(NOT DEFINED INPUT_FILE OR NOT DEFINED OUTPUT_FILE)
  message(FATAL_ERROR "Usage: cmake -DINPUT_FILE=<input> -DOUTPUT_FILE=<output> -P generate_string_literals.cmake")
endif()

file(READ "${INPUT_FILE}" RAW_CONTENT)
# Escape backslashes.
string(REPLACE "\\" "\\\\" RAW_CONTENT_ESCAPED_BS "${RAW_CONTENT}")
# Escape double quotes.
string(REPLACE "\"" "\\\"" RAW_CONTENT_ESCAPED "${RAW_CONTENT_ESCAPED_BS}")
# Replace newlines with newline escape and closing/opening quotes.
string(REPLACE "\n" "\\n\"\n\"" TRANSFORMED_CONTENT "${RAW_CONTENT_ESCAPED}")
set(TRANSFORMED_CONTENT "\"${TRANSFORMED_CONTENT}\"")
file(WRITE "${OUTPUT_FILE}" "${TRANSFORMED_CONTENT}")
