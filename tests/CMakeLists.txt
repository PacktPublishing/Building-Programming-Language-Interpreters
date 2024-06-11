find_package(GTest REQUIRED)

add_library( testlibs
             testlibs/http_message_optrees.hpp
             testlibs/http_message_optrees.cpp
)
target_link_libraries(
    testlibs
    PUBLIC
    networkprotocoldsl
)

find_path(LIBUV_INCLUDE_DIR NAMES uv.h)
find_library(LIBUV_LIBRARIES NAMES uv libuv)
add_library( uv INTERFACE )
target_include_directories( uv INTERFACE ${LIBUV_INCLUDE_DIR} )
target_link_libraries( uv INTERFACE ${LIBUV_LIBRARIES} )

foreach(
    TEST
    001-empty
    002-interpreter-state
    003-literal-and-add-operations
    004-interpreter-global-state
    005-operator-eq
    006-operator-if
    007-opsequence
    008-lexicalpad
    009-recursive-factorial
    010-read-write-static-octets
    011-write-inttoascii
    012-read-int-and-terminator
    013-parse-and-write-http-message
    014-using-with-libuv
)
    add_executable(${TEST}.t ${TEST}.cpp)
    target_link_libraries(
        ${TEST}.t
        PUBLIC
        testlibs
        networkprotocoldsl
        uv
        ${GTEST_BOTH_LIBRARIES}
    )
    add_test(
     NAME ${TEST}
     COMMAND
       ${CMAKE_COMMAND} -E env
       TSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/tsan_suppressions.txt
       $<TARGET_FILE:${TEST}.t>
    )
endforeach()
